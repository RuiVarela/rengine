// __!!rengine_copyright!!__ //

#ifndef __RENGINE_DRAW_RESOURCE_H__
#define __RENGINE_DRAW_RESOURCE_H__

#include <rengine/lang/Lang.h>

namespace rengine
{
	class RenderEngine;

	typedef Uint ResourceId;

	//
	// Generic Draw Resource representation,
	// this should be used to hold rendering objects hold by the render engines
	//
	class DrawResource
	{
	public:
		typedef Uint FlagSet;

		enum DataType
		{
			UnsignedInt					= 0x1401,
			UnsignedByte				= 0x1405,
			Float						= 0x1406
		};

		enum DataFormat
		{
			// Format
			Red							= 0x1903,
			RedGreen					= 0x8227,
			Rgb 						= 0x1907,
			Rgba 						= 0x1908,
			DepthComponent 				= 0x1902,
			StencilIndex 				= 0x1901,

			// Internal Format
			CompressedRed				= 0x8225,
			CompressedRedGreen			= 0x8226,
			Red8						= 0x8229,
			RedGreen8					= 0x822B,
			Red16Float					= 0x822D,
			RedGreen16Float				= 0x822F,

			CompressedRgb				= 0x84ED,
			Rgb8						= 0x8051,
			Rgb16Float					= 0x881B,
			Rgb32Float					= 0x8815,

			CompressedRgba				= 0x84EE,
			Rgba8						= 0x8058,
			Rgba16Float					= 0x881A,
			Rgba32Float					= 0x8814,

			DepthComponent16 			= 0x81A5,
			DepthComponent24 			= 0x81A6,
			DepthComponent32 			= 0x81A7,
			DepthComponent32Float		= 0x8CAC,


			StencilIndex1				= 0x8D46,
			StencilIndex4				= 0x8D47,
			StencilIndex8				= 0x8D48,
			StencilIndex16				= 0x8D49,

			Depth24Stencil8 			= 0x88F0, // Packed Depth Stencil
			Depth32FloatStencil8 		= 0x8CAD  // Packed Depth Stencil
		};


		DrawResource();

		void setId(ResourceId const& id, RenderEngine* engine = 0);
		ResourceId const& getId(RenderEngine* engine = 0) const;

		Bool drawResourceLoaded(RenderEngine* engine = 0) const;


		void clearChangeFlags();
		FlagSet const& changeFlags() const;
		FlagSet& changeFlags();
		Bool isChangeFlagSet(FlagSet const& flag) const;
	private:
		ResourceId resource_id_;
		FlagSet change_flags;
	};


	class VertexBufferObject : public DrawResource
	{
	public:
		VertexBufferObject();
	private:
	};

	class VertexArrayObject : public DrawResource
	{
	public:
		VertexArrayObject();
	private:
	};


	//
	// implementation
	//
	RENGINE_INLINE DrawResource::DrawResource()
		:resource_id_(0), change_flags(0)
	{
	}

	RENGINE_INLINE void DrawResource::clearChangeFlags()
	{
		change_flags = 0;
	}

	RENGINE_INLINE DrawResource::FlagSet& DrawResource::changeFlags()
	{
		return change_flags;
	}

	RENGINE_INLINE DrawResource::FlagSet const& DrawResource::changeFlags() const
	{
		return change_flags;
	}

	RENGINE_INLINE Bool DrawResource::isChangeFlagSet(FlagSet const& flag) const
	{
		return (change_flags & flag) != 0;
	}

	RENGINE_INLINE VertexBufferObject::VertexBufferObject()
	{
	}

	RENGINE_INLINE VertexArrayObject::VertexArrayObject()
	{
	}
}

#endif //__RENGINE_DRAW_RESOURCE_H__

