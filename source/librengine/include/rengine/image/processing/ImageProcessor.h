// __!!rengine_copyright!!__ //

#ifndef __RENGINE_IMAGE_PROCESSOR__
#define __RENGINE_IMAGE_PROCESSOR__

#include <rengine/state/Texture.h>
#include <rengine/geometry/BaseShapes.h>
#include <rengine/state/FrameBuffer.h>
#include <rengine/util/StringTable.h>
#include <rengine/image/Filter.h>

namespace rengine
{
	class ImageProcessor
	{
	public:
		typedef SharedPointer<Texture2D> Connection;
		typedef std::vector<Connection> Connections;

		ImageProcessor();
		virtual ~ImageProcessor();

		void addInput(Connection const& input);
		Connections const& inputs() const;
		Connections& inputs();

		void addOutput(Connection const& output);
		Connections const& outputs() const;
		Connections& outputs();

		virtual void operator()(RenderEngine& render_engine);
		virtual bool initialize();

		void setQuadrilateral(SharedPointer<Quadrilateral> quad);
		SharedPointer<Quadrilateral>const& quadrilateral() const;

		virtual std::string effectFile() const;
	private:
		ImageProcessor(ImageProcessor const& copy);
	protected:
		Connections inputs_;
		Connections outputs_;
		SharedPointer<Quadrilateral> quadrilateral_;
		Bool initialized_;
		StringTable shader_symbols_;
	};

	class ProcessingChain : public ImageProcessor
	{
	public:
		typedef SharedPointer<FrameBuffer> ProcessingBuffer;
		typedef SharedPointer<ImageProcessor> ProcessingStage;
		typedef std::vector<ProcessingStage> ProcessingStages;

		ProcessingChain();

		void addProcessingStage(ProcessingStage const& stage);

		virtual void operator()(RenderEngine& render_engine);
		virtual bool initialize();
	private:
		DrawStates states;
		ProcessingStages stages_;
		ProcessingBuffer ping_;
		ProcessingBuffer pong_;
		SharedPointer<Texture2D> ping_texture_;
	};

	class CopyImageProcessor : public ImageProcessor
	{
	public:
		virtual std::string effectFile() const;
	};

	class GrayscaleImageProcessor : public ImageProcessor
	{
	public:
		virtual std::string effectFile() const;
	};

	class ThresholdImageProcessor : public ImageProcessor
	{
	public:
		ThresholdImageProcessor();
		ThresholdImageProcessor(Real const& threshold);

		virtual bool initialize();
		Real threshold() const;
		void setThreshold(Real const& threshold);

		Vector4D thresholdValue() const;
		void setThresholdValue(Vector4D const& value);

		virtual std::string effectFile() const;
	private:
		Real threshold_;
		Vector4D value_;
	};

	class BinarizationImageProcessor : public ImageProcessor
	{
	public:
		BinarizationImageProcessor();
		BinarizationImageProcessor(Real const& threshold);

		virtual bool initialize();
		Real threshold() const;
		void setThreshold(Real const& threshold);

		virtual std::string effectFile() const;
	private:
		Real threshold_;
		Vector4D value_;
	};

	class KernelImageProcessor : public ImageProcessor
	{
	public:
		KernelImageProcessor(SharedPointer<Kernel> kernel);

		virtual bool initialize();

		virtual std::string effectFile() const;
	private:
		SharedPointer<Kernel> kernel_;
	};

	class SeparableKernelImageProcessor : public ImageProcessor
	{
	public:
		SeparableKernelImageProcessor(SharedPointer<Kernel> kernel0, SharedPointer<Kernel> kernel1);

		virtual bool initialize();

		virtual std::string effectFile() const;
	private:
		SharedPointer<Kernel> kernel0_;
		SharedPointer<Kernel> kernel1_;
	};

	//
	// ImageProcessor
	//
	RENGINE_INLINE ImageProcessor::ImageProcessor()
	:initialized_(false)
	{
	}

	RENGINE_INLINE void ImageProcessor::addInput(Connection const& input)
	{
		inputs_.push_back(input);
	}

	RENGINE_INLINE ImageProcessor::Connections const& ImageProcessor::inputs() const
	{
		return inputs_;
	}

	RENGINE_INLINE ImageProcessor::Connections& ImageProcessor::inputs()
	{
		return inputs_;
	}

	RENGINE_INLINE void ImageProcessor::addOutput(Connection const& output)
	{
		outputs_.push_back(output);
	}

	RENGINE_INLINE ImageProcessor::Connections const& ImageProcessor::outputs() const
	{
		return outputs_;
	}

	RENGINE_INLINE ImageProcessor::Connections& ImageProcessor::outputs()
	{
		return outputs_;
	}

	RENGINE_INLINE void ImageProcessor::setQuadrilateral(SharedPointer<Quadrilateral> quad)
	{
		quadrilateral_ = quad;
	}

	RENGINE_INLINE SharedPointer<Quadrilateral> const& ImageProcessor::quadrilateral() const
	{
		return quadrilateral_;
	}


	//
	// ThresholdImageProcessor
	//
	RENGINE_INLINE Real ThresholdImageProcessor::threshold() const
	{
		return threshold_;
	}

	RENGINE_INLINE void ThresholdImageProcessor::setThreshold(Real const& threshold)
	{
		threshold_ = threshold;
	}

	RENGINE_INLINE Vector4D ThresholdImageProcessor::thresholdValue() const
	{
		return value_;
	}

	RENGINE_INLINE void ThresholdImageProcessor::setThresholdValue(Vector4D const& value)
	{
		value_ = value;
	}

	//
	// BinarizationImageProcessor
	//

	RENGINE_INLINE Real BinarizationImageProcessor::threshold() const
	{
		return threshold_;
	}

	RENGINE_INLINE void BinarizationImageProcessor::setThreshold(Real const& threshold)
	{
		threshold_ = threshold;
	}



} // namespace end

#endif //__RENGINE_IMAGE_PROCESSOR__
