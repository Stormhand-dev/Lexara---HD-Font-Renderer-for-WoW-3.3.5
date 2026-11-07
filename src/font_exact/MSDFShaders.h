#pragma once

// These shaders replace the game's UI shader pair, so they run for ALL UI
// drawing (textures as well as text), not only for MSDF glyphs.
//
// The vertex shader must therefore be a pure passthrough and must not read
// any constant register the game itself uses. It previously read c23 to
// decide whether to fold the UV signs, but the game writes VS c23 constantly
// for its own world/model shaders. A stale non-zero value there made UI
// textures with negative texcoords (e.g. the rotated world map player arrow)
// get abs()'d, which mirrored and stretched them.
//
// All MSDF decoding now happens in the pixel shader, driven by PS c23, which
// the game never writes (Lexara resets it to zero after every font batch).

inline auto* vertexShaderHLSL = R"(
	uniform float4x4 WorldViewProj;

	struct VS_IN {
		float4 pos  : POSITION0;
		float4 col  : COLOR0;
		float2 uv0  : TEXCOORD0;
	};

	struct VS_OUT {
		float4 hpos : POSITION;
		float4 col  : COLOR0;
		float2 uv0  : TEXCOORD0;
	};

	VS_OUT main(VS_IN IN) {
		VS_OUT OUT;
		OUT.hpos = mul(IN.pos, WorldViewProj);
		OUT.col  = IN.col;
		OUT.uv0  = IN.uv0; // untouched: signed UVs are decoded in the pixel shader
		return OUT;
	}
)";

inline auto* pixelShaderHLSL = R"(
	sampler2D gameTexture : register(s0);

	sampler2D sdfAtlas0   : register(s12);
	sampler2D sdfAtlas1   : register(s13);
	sampler2D sdfAtlas2   : register(s14);
	sampler2D sdfAtlas3   : register(s15);

	float4 control : register(c23); // font size, outline mode, spread, atlas size

	struct PS_IN {
		float4 col : COLOR0;
		float2 uv0 : TEXCOORD0;
	};

	float median(float r, float g, float b) {
		return max(min(r, g), min(max(r, g), b));
	}

	float4 main(PS_IN IN) : COLOR {
		if (control.x < 1.0f) return tex2D(gameTexture, IN.uv0) * IN.col;

		// MSDF glyph: the atlas page is encoded in the UV sign bits.
		// All four vertices of a glyph quad share the same signs, so the
		// interpolated value keeps them.
		float bit0 = (IN.uv0.x < 0.0f) ? 1.0f : 0.0f;
		float bit1 = (IN.uv0.y < 0.0f) ? 2.0f : 0.0f;
		int atlasPage = int(bit0 + bit1 + 0.5f);

		float2 uv = abs(IN.uv0);

		float outlineHint = control.y;
		float fontSize = control.x;
		float outlinePx = 0.0f;

		if (outlineHint >= 1.5f) {
			outlinePx = max(2.75f, pow(fontSize, 0.150f) * 1.6f);
		} else if (outlineHint >= 0.5f) {
			outlinePx = max(1.50f, pow(fontSize, 0.075f) * 1.5f);
		}

		float4 sample;
		if (atlasPage == 0) sample = tex2D(sdfAtlas0, uv);
		else if (atlasPage == 1) sample = tex2D(sdfAtlas1, uv);
		else if (atlasPage == 2) sample = tex2D(sdfAtlas2, uv);
		else sample = tex2D(sdfAtlas3, uv);

		float sd = median(sample.r, sample.g, sample.b);
		float screenPxRange = (control.z / max(max(fwidth(uv.x), fwidth(uv.y)) * control.a, 1e-6)) * (1.0f - min(0.3f, fontSize * 0.0035f)); // smoother edges for larger text
		float opacity = saturate((sd - 0.5f) * screenPxRange + 0.5f);

		if (outlinePx > 0.0f) {
			// *5 of the outline msdf, mult has to match alpha-channel range
			return float4(
				lerp(float3(0.0f, 0.0f, 0.0f), IN.col.rgb, opacity),
				max(opacity, saturate((sample.a - 0.5f) * screenPxRange * 5.0f + outlinePx)) * IN.col.a
			);
		}
		return float4(IN.col.rgb, opacity * IN.col.a);
	}
)";
