// __!!rengine_copyright!!__ //

#ifndef __RENGINE_DRAWABLE_H__
#define __RENGINE_DRAWABLE_H__

#include <rengine/state/DrawStates.h>
#include <vector>

namespace rengine
{
	class RenderEngine;

	class Drawable
	{
	public:

		enum DrawMode
		{
			StaticDraw		= 0x88E4,
			DynamicDraw		= 0x88E8,
			StreamDraw		= 0x88E0
		};

		typedef Uint IndexType;
		typedef std::vector<IndexType> IndexVector;


		Drawable();
		virtual ~Drawable();

		// initializes draw resources
		virtual void prepareDrawing(RenderEngine& render_engine);
		// release draw resources
		virtual void unprepareDrawing(RenderEngine& render_engine);
		// update uniforms, this is called before applying states
		virtual void updateUniforms(RenderEngine& render_engine);
		// draws the object
		virtual void draw(RenderEngine& render_engine);


		virtual void setDrawMode(DrawMode const draw_mode);
		DrawMode const& drawMode() const;

		virtual void setNeedsDataRefresh(Bool const value = true);
		Bool needsDataRefresh() const;

		Bool hasDrawStates() const;

		//
		// This method returns the object draw_states
		//
		SharedPointer<DrawStates> const& getDrawStates() const;
		void setDrawStates(SharedPointer<DrawStates> const& states);

		//
		// The same as above but,
		// if draw_states is null creates a new DrawStates object
		//
		SharedPointer<DrawStates> states();

	private:
		DrawMode draw_mode_;

	protected:
		SharedPointer<DrawStates> draw_states;
		Bool needs_prepare_rendering;
		Bool needs_data_refresh;
	};

	//
	// Implementation
	//


	RENGINE_INLINE Drawable::Drawable() :
		draw_mode_(StaticDraw), needs_prepare_rendering(false), needs_data_refresh(false)
	{
	}

	RENGINE_INLINE Drawable::~Drawable()
	{
	}

	RENGINE_INLINE void Drawable::setDrawMode(DrawMode const draw_mode)
	{
		draw_mode_ = draw_mode;
	}

	RENGINE_INLINE Drawable::DrawMode const& Drawable::drawMode() const
	{
		return draw_mode_;
	}

	RENGINE_INLINE void Drawable::setNeedsDataRefresh(Bool const value)
	{
		needs_data_refresh = value;
	}

	RENGINE_INLINE Bool Drawable::needsDataRefresh() const
	{
		return needs_data_refresh;
	}

	RENGINE_INLINE SharedPointer<DrawStates> const& Drawable::getDrawStates() const
	{
		return draw_states;
	}

	RENGINE_INLINE void Drawable::setDrawStates(SharedPointer<DrawStates> const& states)
	{
		draw_states = states;
	}

	RENGINE_INLINE SharedPointer<DrawStates> Drawable::states()
	{
		if (!draw_states)
		{
			draw_states = new DrawStates();
		}

		return draw_states;
	}

	RENGINE_INLINE Bool Drawable::hasDrawStates() const
	{
		return (draw_states);
	}

}

#endif //__RENGINE_DRAWABLE_H__
