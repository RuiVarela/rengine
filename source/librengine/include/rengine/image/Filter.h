// __!!rengine_copyright!!__ //

#ifndef __RENGINE_MATH_FILTER_H__
#define __RENGINE_MATH_FILTER_H__

#include <rengine/math/Math.h>

#include <string>

namespace rengine
{
	// Based on the public domain code by castanyo@yahoo.es

	class Filter
	{
	public:
		enum SamplingMethod
		{
			Delta,
			Box,
			Triangle
		};

		Filter(Real const& filter_width);
		virtual ~Filter();
		virtual float evaluate(Real const& x) const = 0;
		Real const& width() const;

		Real sample(SamplingMethod const& method, Real const& x0, Real const& x1, Uint const& samples) const;
		Real sampleDelta(Real const& x0, Real const& x1) const;
		Real sampleBox(Real const& x0, Real const& x1, Uint const& samples) const;
		Real sampleTriangle(Real const& x0, Real const& x1, Uint const& samples) const;
	protected:
		Real width_;
	};

	class BoxFilter : public Filter
	{
	public:
		BoxFilter();
		BoxFilter(Real const& width);
		virtual float evaluate(Real const& x) const;
	};

	class TriangleFilter : public Filter
	{
	public:
		TriangleFilter();
		TriangleFilter(Real const& width);
		virtual float evaluate(Real const& x) const;
	};

	class QuadraticFilter : public Filter
	{
	public:
		QuadraticFilter();
		QuadraticFilter(Real const& width);
		virtual float evaluate(Real const& x) const;
	};

	class CubicFilter : public Filter
	{
	public:
		CubicFilter();
		CubicFilter(Real const& width);
		virtual float evaluate(Real const& x) const;
	};

	class BsplineFilter : public Filter
	{
	public:
		BsplineFilter();
		virtual float evaluate(Real const& x) const;
	};

	class MitchellFilter : public Filter
	{
	public:
		MitchellFilter();
		virtual Real evaluate(Real const& x) const;

		void setParameters(Real const& b, Real const& c);
	private:
		float p0, p2, p3;
		float q0, q1, q2, q3;
	};

	class LanczosFilter : public Filter
	{
	public:
		LanczosFilter();
		LanczosFilter(Real const& width);
		virtual Real evaluate(Real const& x) const;
	};

	class SincFilter : public Filter
	{
	public:
		SincFilter();
		SincFilter(Real const& width);
		virtual Real evaluate(Real const& x) const;
	};

	class KaiserFilter : public Filter
	{
	public:
		KaiserFilter();
		KaiserFilter(Real const& width);
		virtual Real evaluate(Real const& x) const;

		void setParameters(Real alpha, Real stretch);
	private:
		Real alpha;
		Real stretch;
	};

	class GaussianFilter : public Filter
	{
	public:
		GaussianFilter();
	    //standard_deviation <= 0.5
		GaussianFilter(Real const& filter_width, Real const& standard_deviation);
		virtual Real evaluate(Real const& x) const;
	private:
		Real standard_deviation_;
	};

	class Kernel
	{
	public:
		enum Orientation
		{
			None		= 0,
			Vertical	= 1,
			Horizontal	= 2
		};

		Kernel(Uint const& size, Uint const& data_size);
		~Kernel();

		Kernel(Kernel const& copy);
		Kernel& operator=(Kernel constother);
		void swap(Kernel& other);

		Real* data();
		Uint size() const;
		Uint dataSize() const;
		Filter::SamplingMethod samplingMethod() const;
		void setSamplingMethod(Filter::SamplingMethod const sampling_method);

		virtual Uint dimensions() const;
		virtual std::string toString() const;

		void normalize();

		void add(Kernel const& other);
		void subtract(Kernel const& other);
		void multiply(Kernel const& other);

		void setOrientation(Orientation const& kernel_orientation);
		Orientation orientation() const;
	protected:
		Real* data_;
		Uint size_;
		Uint data_size_;
		Filter::SamplingMethod sampling_method_;
		Orientation orientation_;
	};

	class Kernel1D : public Kernel
	{
	public:
		Kernel1D(Uint const& size);
		Kernel1D(Filter const& filter, Uint const& size, Real const& scale, Uint const& samples = 32);

		virtual Uint dimensions() const;

		//scale 1 fits the kernel size to the filter width
		bool build(Filter const& filter, Real const& scale, Uint const& samples = 32);
		Real const& value(Uint const x) const;
		Real& value(Uint const x);

		virtual std::string toString() const;

		static SharedPointer<Kernel1D> gaussian(Uint const kernel_size, Orientation const& orientation, Real const& standard_deviation = 0.5);
	};

	class Kernel2D : public Kernel
	{
	public:
		//scale 1 fits the kernel size to the filter width
		Kernel2D(Uint const& size);
		Kernel2D(Filter const& filter, Uint const& size, Real const& scale, Uint const& samples = 32);

		virtual Uint dimensions() const;

		bool build(Filter const& filter, Real const& scale, Uint const& samples = 32);
		Real const& value(Uint const x, Uint const y) const;
		Real& value(Uint const x, Uint const y);

		virtual std::string toString() const;

		void transpose();
		void rotateRight();
		void rotateLeft();

		void setAsLaplacian(Bool const with_diagonals = true);
		void setAsSobel(Orientation const& kernel_orientation = Vertical);
		void setAsPrewitt(Orientation const& kernel_orientation = Vertical);
		void setAsScharr(Orientation const& kernel_orientation = Vertical);

		static SharedPointer<Kernel2D> laplacian(Bool const with_diagonals = true);
		static SharedPointer<Kernel2D> sobel(Uint const kernel_size, Orientation const& kernel_orientation = Vertical);
		static SharedPointer<Kernel2D> prewitt(Orientation const& kernel_orientation = Vertical);
		static SharedPointer<Kernel2D> scharr(Orientation const& kernel_orientation = Vertical);

		static SharedPointer<Kernel2D> gaussian(Uint const kernel_size, Real const& standard_deviation = 0.5);
	};



	//
	// Filter
	//
	RENGINE_INLINE Filter::Filter(Real const& filter_width)
	:width_(filter_width)
	{}

	RENGINE_INLINE Filter::~Filter()
	{}

	RENGINE_INLINE Real const& Filter::width() const
	{
		return width_;
	}

	//
	// Box
	//
	RENGINE_INLINE BoxFilter::BoxFilter()
	:Filter(0.5f)
	{}

	RENGINE_INLINE BoxFilter::BoxFilter(Real const& filter_width)
	:Filter(filter_width)
	{}

	//
	// Triangle
	//
	RENGINE_INLINE TriangleFilter::TriangleFilter()
	:Filter(1.0f)
	{}

	RENGINE_INLINE TriangleFilter::TriangleFilter(Real const& filter_width)
	:Filter(filter_width)
	{}

	//
	// Quadratic
	//
	RENGINE_INLINE QuadraticFilter::QuadraticFilter()
	:Filter(1.0f)
	{}

	RENGINE_INLINE QuadraticFilter::QuadraticFilter(Real const& filter_width)
	:Filter(filter_width)
	{}

	//
	// Quadratic
	//
	RENGINE_INLINE CubicFilter::CubicFilter()
	:Filter(1.0f)
	{}

	RENGINE_INLINE CubicFilter::CubicFilter(Real const& filter_width)
	:Filter(filter_width)
	{}

	//
	// Bspline Filter
	//
	RENGINE_INLINE BsplineFilter::BsplineFilter()
	:Filter(2.0f)
	{}

	//
	// MitchellFilter
	//
	RENGINE_INLINE MitchellFilter::MitchellFilter()
	:Filter(2.0f)
	{
		setParameters(1.0f/3.0f, 1.0f/3.0f);
	}

	//
	// LanczosFilter
	//
	RENGINE_INLINE LanczosFilter::LanczosFilter()
	:Filter(3.0f)
	{}

	RENGINE_INLINE LanczosFilter::LanczosFilter(Real const& filter_width)
	:Filter(filter_width)
	{}

	//
	// LanczosFilter
	//
	RENGINE_INLINE SincFilter::SincFilter()
	:Filter(1.0f)
	{}

	RENGINE_INLINE SincFilter::SincFilter(Real const& filter_width)
	:Filter(filter_width)
	{}

	//
	// LanczosFilter
	//
	RENGINE_INLINE KaiserFilter::KaiserFilter()
	:Filter(1.0f)
	{
		setParameters(4.0f, 1.0f);
	}

	RENGINE_INLINE KaiserFilter::KaiserFilter(Real const& filter_width)
	:Filter(filter_width)
	{
		setParameters(4.0f, 1.0f);
	}

	RENGINE_INLINE void KaiserFilter::setParameters(Real alpha, Real stretch)
	{
		this->alpha = alpha;
		this->stretch = stretch;
	}

	//
	//GaussianFilter
	//
	RENGINE_INLINE GaussianFilter::GaussianFilter()
	:Filter(0.5f), standard_deviation_(0.4f)
	{}

	RENGINE_INLINE GaussianFilter::GaussianFilter(Real const& filter_width, Real const& standard_deviation)
	:Filter(filter_width), standard_deviation_(standard_deviation)
	{}

	//
	// KernelD
	//
	RENGINE_INLINE Kernel::Kernel(Uint const& size, Uint const& data_size)
	:data_(new Real[data_size]), size_(size), data_size_(data_size), sampling_method_(Filter::Box), orientation_(None)
	{}

	RENGINE_INLINE Kernel::~Kernel()
	{
		delete[](data_);
	}


	RENGINE_INLINE Filter::SamplingMethod Kernel::samplingMethod() const
	{
		return sampling_method_;
	}

	RENGINE_INLINE void Kernel::setSamplingMethod(Filter::SamplingMethod const sampling_method)
	{
		sampling_method_ = sampling_method;
	}

	RENGINE_INLINE void Kernel::setOrientation(Orientation const& kernel_orientation)
	{
		orientation_ = kernel_orientation;
	}

	RENGINE_INLINE Kernel::Orientation Kernel::orientation() const
	{
		return orientation_;
	}

	RENGINE_INLINE Real* Kernel::data()
	{
		return data_;
	}

	RENGINE_INLINE Uint Kernel::size() const
	{
		return size_;
	}

	RENGINE_INLINE Uint Kernel::dataSize() const
	{
		return data_size_;
	}

	RENGINE_INLINE Uint Kernel::dimensions() const
	{
		return 0;
	}

	RENGINE_INLINE std::string Kernel::toString() const
	{
		return "";
	}

	//
	// Kernel1D
	//
	RENGINE_INLINE Uint Kernel1D::dimensions() const
	{
		return 1;
	}

	RENGINE_INLINE SharedPointer<Kernel1D> Kernel1D::gaussian(Uint const kernel_size, Orientation const& orientation, Real const& standard_deviation)
	{
		GaussianFilter filter(0.5f, standard_deviation);
		SharedPointer<Kernel1D> kernel = new Kernel1D(filter, kernel_size, 1.0f, 32);
		kernel->setOrientation(orientation);
		return kernel;
	}

	//
	// Kernel2D
	//
	RENGINE_INLINE Uint Kernel2D::dimensions() const
	{
		return 2;
	}

	RENGINE_INLINE SharedPointer<Kernel2D> Kernel2D::laplacian(Bool const with_diagonals)
	{
		SharedPointer<Kernel2D> kernel = new Kernel2D(3);
		kernel->setAsLaplacian(with_diagonals);
		return kernel;
	}

	RENGINE_INLINE SharedPointer<Kernel2D> Kernel2D::sobel(Uint const kernel_size, Orientation const& kernel_orientation)
	{
		SharedPointer<Kernel2D> kernel = new Kernel2D(kernel_size);
		kernel->setAsSobel(kernel_orientation);
		return kernel;
	}

	RENGINE_INLINE SharedPointer<Kernel2D> Kernel2D::prewitt(Orientation const& kernel_orientation)
	{
		SharedPointer<Kernel2D> kernel = new Kernel2D(3);
		kernel->setAsPrewitt(kernel_orientation);
		return kernel;
	}

	RENGINE_INLINE SharedPointer<Kernel2D> Kernel2D::scharr(Orientation const& kernel_orientation)
	{
		SharedPointer<Kernel2D> kernel = new Kernel2D(3);
		kernel->setAsScharr(kernel_orientation);
		return kernel;
	}

	RENGINE_INLINE SharedPointer<Kernel2D> Kernel2D::gaussian(Uint const kernel_size, Real const& standard_deviation)
	{
		GaussianFilter filter(0.5f, standard_deviation);
		SharedPointer<Kernel2D> kernel = new Kernel2D(filter, kernel_size, 1.0f, 32);
		return kernel;
	}

} // namespace rengine


#endif //__RENGINE_MATH_FILTER_H__
