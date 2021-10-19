#pragma once

struct DDSHeader {
	enum Flags {
		CAPS = 0x00000001,
		HEIGHT = 0x00000002,
		WIDTH = 0x00000004,
		PITCH = 0x00000008,
		PIXELFORMAT = 0x00001000,
		MIPMAPCOUNT = 0x00020000,
		LINEARSIZE = 0x00080000,
		DEPTH = 0x00800000
	};

	struct PixelFormat {
		enum Flags {
			ALPHAPIXELS = 0x00000001,
			FOURCC = 0x00000004,
			RGB = 0x00000040
		};

		unsigned size = 0;
		unsigned flags = 0;
		unsigned fourCC = 0;
		unsigned rgbBitCount = 0;
		unsigned rBitMask = 0;
		unsigned gBitMask = 0;
		unsigned bBitMask = 0;
		unsigned alphaBitMask = 0;
	};

	struct Caps {
		enum Caps1 {
			COMPLEX = 0x00000008,
			TEXTURE = 0x00001000,
			MIPMAP = 0x00400000
		};

		enum Caps2 {
			CUBEMAP = 0x00000200,
			CUBEMAP_POSITIVEX = 0x00000400,
			CUBEMAP_NEGATIVEX = 0x00000800,
			CUBEMAP_POSITIVEY = 0x00001000,
			CUBEMAP_NEGATIVEY = 0x00002000,
			CUBEMAP_POSITIVEZ = 0x00004000,
			CUBEMAP_NEGATIVEZ = 0x00008000,
			VOLUME = 0x00200000
		};

		unsigned caps1 = 0;
		unsigned caps2 = 0;
		unsigned DDSX = 0;
		unsigned reserved = 0;
	};

	unsigned magic = 0;
	unsigned size = 0;
	unsigned flags = 0;
	unsigned height = 0;
	unsigned width = 0;
	unsigned pitchOrLinearSize = 0;
	unsigned depth = 0;
	unsigned mipMapCount = 0;
	unsigned reserved1[11] = {0};

	PixelFormat pixelFormat;

	Caps caps;

	unsigned reserved2 = 0;
};
