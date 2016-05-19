// __!!rengine_copyright!!__ //

#include <rengine/image/Filter.h>
#include <rengine/math/Math.h>
#include <rengine/string/String.h>

#include <algorithm>
#include <iostream>

namespace rengine
{
	// Bessel function of the first kind from Jon Blow's article.
	// http://mathworld.wolfram.com/BesselFunctionoftheFirstKind.html
	// http://en.wikipedia.org/wiki/Bessel_function
	inline static Real bessel0(Real x)
	{
		const Real EPSILON_RATIO = 1e-6f;
		Real xh, sum, pow, ds;
		Int k;

		xh = 0.5f * x;
		sum = 1.0f;
		pow = 1.0f;
		k = 0;
		ds = 1.0;
		while (ds > sum * EPSILON_RATIO) {
			++k;
			pow = pow * (xh / k);
			ds = pow * pow;
			sum = sum + ds;
		}

		return sum;
	}

	Real Filter::sample(SamplingMethod const& method, Real const& x0, Real const& x1, Uint const& samples) const
	{
		switch (method)
		{
			case Delta:
			return sampleDelta(x0, x1);
				break;
			case Box:
			return sampleBox(x0, x1, samples);
				break;
			case Triangle:
			return sampleTriangle(x0, x1, samples);
				break;
			default:
				break;

		}

		return 0.0f;
	}

	Real Filter::sampleDelta(Real const& x0, Real const& x1) const
	{
		Real centroid = x0 + ((x1 - x0) / 2.0f);
		return evaluate(centroid);
	}

	Real Filter::sampleBox(Real const& x0, Real const& x1, Uint const& samples) const
	{
		Real sum = 0.0f;
		Real delta_x = (x1 - x0) / Real(samples - 1);

		for(Uint sample = 0; sample != samples; sample++)
		{
			Real position = x0 + Real(sample) * delta_x;
			//std::cout << "delta_x " << delta_x << " Position " << position << std::endl;

			Real value = evaluate(position);
			sum += value;
		}

		return sum / Real(samples);
	}

	Real Filter::sampleTriangle(Real const& x0, Real const& x1, Uint const& samples) const
	{
		Real sum = 0.0f;
		Real delta_x = (x1 - x0) / Real(samples - 1);

		for(Uint sample = 0; sample != samples; sample++)
		{
			Real delta = Real(sample) * delta_x;
			Real value = evaluate(x0 + delta);

			Real weight = (delta / (x1 - x0)) * 2.0f;
			if (weight > 1.0f)
			{
				weight = 2.0 - weight;
			}

			sum += value * weight;
		}

		return sum / Real(samples);
	}

	Real BoxFilter::evaluate(Real const& x) const
	{
		if (absolute(x) <= width())
		{
			return 1.0f;
		}

		return 0.0f;
	}

	Real TriangleFilter::evaluate(Real const& x) const
	{
		Real absolute_x = absolute(x);
		if (absolute_x < width())
		{
			return width() - absolute_x;
		}

		return 0.0f;
	}

	Real QuadraticFilter::evaluate(Real const& x) const
	{
		// 1 - d^2/r^2;
		Real absolute_x = absolute(x);
		if (absolute_x < width())
		{
			return (width() * width()) - (absolute_x * absolute_x);
		}

		return 0.0f;
	}

	Real CubicFilter::evaluate(Real const& x) const
	{
		// f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1

		//1 - 3d^2/r^2 + 2d^3/r^3;
		Real absolute_x = absolute(x);
		if(absolute_x < width())
		{
			Real absolute_x2 = absolute_x * absolute_x;
			Real width_2 = width() * width();

			return 1.0f - (3.0f * absolute_x2 / width_2) + (2.0f * (absolute_x2 * absolute_x) / (width_2 * width()));
		}

		return 0.0f;
	}

	Real BsplineFilter::evaluate(Real const& x) const
	{
		Real absolute_x = absolute(x);
	    if(absolute_x < 1.0f)
	    {
	    	return (4.0f + absolute_x * absolute_x * (-6.0f + absolute_x * 3.0f)) / 6.0f;
	    }

	    if(absolute_x < 2.0f)
	    {
	    	Real t = 2.0f - absolute_x;
	    	return t * t * t / 6.0f;
	    }
	    return 0.0f;
	}

	Real MitchellFilter::evaluate(Real const& x) const
	{
		Real absolute_x = absolute(x);
		if (absolute_x < 1.0f)
		{
			return p0 + absolute_x * absolute_x * (p2 + absolute_x * p3);
		}

		if (absolute_x < 2.0f)
		{
			return q0 + absolute_x * (q1 + absolute_x * (q2 + absolute_x * q3));
		}
		return 0.0f;
	}

	void MitchellFilter::setParameters(Real const& b, Real const& c)
	{
		p0 = (6.0f -  2.0f * b) / 6.0f;
		p2 = (-18.0f + 12.0f * b + 6.0f * c) / 6.0f;
		p3 = (12.0f - 9.0f * b - 6.0f * c) / 6.0f;
		q0 = (8.0f * b + 24.0f * c) / 6.0f;
		q1 = (-12.0f * b - 48.0f * c) / 6.0f;
		q2 = (6.0f * b + 30.0f * c) / 6.0f;
		q3 = (-b - 6.0f * c) / 6.0f;
	}

	Real LanczosFilter::evaluate(Real const& x) const
	{
		Real absolute_x = absolute(x);
		if (absolute_x < width())
		{
			return sinc(pi_angle * x) * sinc(pi_angle * x / width());
		}
		return 0.0f;
	}

	Real SincFilter::evaluate(Real const& x) const
	{
		Real absolute_x = absolute(x);
		if (absolute_x < width())
		{
			return sinc(pi_angle * x);
		}
		return 0.0f;
	}

	Real KaiserFilter::evaluate(Real const& x) const
	{
		Real const sinc_value = sinc(pi_angle * x * stretch);
		Real const t = x / width();

		if ((1 - t * t) >= 0)
		{
			return sinc_value * bessel0(alpha * sqrtf(1 - t * t)) / bessel0(alpha);
		}

		return 0;
	}

	Real GaussianFilter::evaluate(Real const& x) const
	{
		Real absolute_x = absolute(x);
		if (absolute_x <= width())
		{
			return exp(-0.5f * pow(absolute_x / (standard_deviation_ * width() * 0.5f), 2.0f) );
		}

		return 0.0f;
	}


	//
	// Kernel
	//
	Kernel::Kernel(Kernel const& copy)
	:data_(new Real[copy.data_size_]), size_(copy.size_), data_size_(copy.data_size_), sampling_method_(copy.sampling_method_)
	{
		std::copy(copy.data_, copy.data_ + copy.data_size_, data_);
	}

	void Kernel::swap(Kernel& other)
	{
		std::swap(size_, other.size_);
		std::swap(data_size_, other.data_size_);
		std::swap(sampling_method_, other.sampling_method_);
		std::swap(data_, other.data_);
	}

	Kernel& Kernel::operator=(Kernel other)
	{
		swap(other);
		return *this;
	}

	void Kernel::normalize()
	{
		Real total = 0.0f;

		for (Uint i = 0; i != data_size_; i++)
		{
			total += data_[i];
		}

		Real inverse = 1.0f / total;
		for (Uint i = 0; i != data_size_; i++)
		{
			data_[i] *= inverse;
		}
	}

	void Kernel::add(Kernel const& other)
	{
		RENGINE_ASSERT(other.data_size_ >= data_size_);

		for (Uint i = 0; i != data_size_; i++)
		{
			data_[i] += other.data_[i];
		}
	}

	void Kernel::subtract(Kernel const& other)
	{
		RENGINE_ASSERT(other.data_size_ >= data_size_);

		for (Uint i = 0; i != data_size_; i++)
		{
			data_[i] -= other.data_[i];
		}
	}

	void Kernel::multiply(Kernel const& other)
	{
		RENGINE_ASSERT(other.data_size_ >= data_size_);

		for (Uint i = 0; i != data_size_; i++)
		{
			data_[i] *= other.data_[i];
		}
	}

	//
	// Kernel1D
	//
	Kernel1D::Kernel1D(Uint const& size)
	:Kernel(size, size)
	{}

	Kernel1D::Kernel1D(Filter const& filter, Uint const& size, Real const& scale, Uint const& samples)
	:Kernel(size, size)
	{
		build(filter, scale, samples);
	}

	Real const& Kernel1D::value(Uint const x) const
	{
		RENGINE_ASSERT(x < size_);
		return data_[x];
	}

	Real& Kernel1D::value(Uint const x)
	{
		RENGINE_ASSERT(x < size_);
		return data_[x];
	}

	std::string Kernel1D::toString() const
	{
		return separateElements(data_, size(), ", ");
	}

	bool Kernel1D::build(Filter const& filter, Real const& scale, Uint const& samples)
	{
		RENGINE_ASSERT(scale > 0.0f);
		RENGINE_ASSERT(size_ > 1);
		RENGINE_ASSERT((size_ % 2) != 0);

		Real const offset = floorf( Real(size_) / 2.0f );

		//adjust the filter width to fit entirely on the kernel size
		Real scaling = (filter.width() * 2.0f) / Real(size_);

		//rescale based on the user value
		scaling *= scale;

		Real total = 0.0f;
		for (Uint i = 0; i != size_; i++)
		{
			//0.5 - pixel start
			Real p0 = (Real(i + 0) - 0.5f - offset) * scaling;
			Real p1 = (Real(i + 1) - 0.5f - offset) * scaling;

			Real sample = filter.sample(samplingMethod(), p0, p1, samples);
			value(i) = sample;
			total += sample;

			//std::cout << "Kernel1D sample [" << p0 << ", " << p1 << "] " << sample << std::endl;
		}

		Real inverse = 1.0f / total;
		for (Uint i = 0; i != size_; i++)
		{
			data_[i] *= inverse;
		}

		return true;
	}

	Kernel2D::Kernel2D(Uint const& size)
	:Kernel(size, size * size)
	{}

	Kernel2D::Kernel2D(Filter const& filter, Uint const& size, Real const& scale, Uint const& samples)
	:Kernel(size, size * size)
	{
		build(filter, scale, samples);
	}

	Real const& Kernel2D::value(Uint const x, Uint const y) const
	{
		RENGINE_ASSERT(x < size_);
		RENGINE_ASSERT(y < size_);

		return data_[y * size_ + x];
	}

	Real& Kernel2D::value(Uint const x, Uint const y)
	{
		RENGINE_ASSERT(x < size_);
		RENGINE_ASSERT(y < size_);

		return data_[y * size_ + x];
	}

	std::string Kernel2D::toString() const
	{
		std::string output;

		for (Uint y = 0; y != size_; y++)
		{
			if (output != "")
			{
				output += ",\n";
			}
			//output += separateElements(&data_[ (size_ - 1 - y) * size()], size(), ", ");
			output += separateElements(&data_[y * size()], size(), ", ");
		}
		return output;
	}

	bool Kernel2D::build(Filter const& filter, Real const& scale, Uint const& samples)
	{
		RENGINE_ASSERT(scale > 0.0f);
		RENGINE_ASSERT(size_ > 1);
		RENGINE_ASSERT((size_ % 2) != 0);

		Real const offset = floorf( Real(size_) / 2.0f );

		//adjust the filter width to fit entirely on the kernel size
		Real diagonal_size = sqrt( square(Real(size_)) + square(Real(size_)) );
		Real scaling = (filter.width() * 2.0f) / Real(diagonal_size);

		//rescale based on the user value
		scaling *= scale;

		Real total = 0.0f;
		for (Uint x = 0; x != size_; x++)
		{
			for (Uint y = 0; y != size_; y++)
			{
				Real x_distance = (Real(x) - offset) * scaling;
				Real y_distance = (Real(y) - offset) * scaling;

				Real middle = sqrt( square(x_distance) + square(y_distance) );

				Real p0 = middle - 0.5f * scaling;
				Real p1 = middle + 0.5f * scaling;;

				Real sample = filter.sample(samplingMethod(), p0, p1, samples);
				value(x, y) = sample;
				total += sample;

				//std::cout << "Kernel2D sample [" << x << ", " << y << "] " <<  p0 << " , " << p1 << "  | "<< sample << std::endl;
			}
		}

		Real inverse = 1.0f / total;
		for (Uint i = 0; i != size_ * size_; i++)
		{
			data_[i] *= inverse;
		}

		return true;
	}

	void Kernel2D::transpose()
	{
		Real tmp = 0.0f;

		for (Uint x = 0; x != size_; x++)
		{
			for (Uint y = x + 1; y != size_; y++)
			{
				fastSwap(value(x, y), value(y, x), tmp);
			}
		}
	}

	void Kernel2D::rotateRight()
	{
		Real* rotated = new Real[size_ * size_];

		for (Uint x = 0; x != size_; x++)
		{
			for (Uint y = 0; y != size_; y++)
			{
				rotated[(size_ - x - 1) * size_ + y] = data_[y * size_ + x];
			}
		}

		slowSwap(rotated, data_);
		delete[](rotated);
	}

	void Kernel2D::rotateLeft()
	{
		Real* rotated = new Real[size_ * size_];

		for (Uint x = 0; x != size_; x++)
		{
			for (Uint y = 0; y != size_; y++)
			{
				rotated[x * size_ + (size_ - y - 1)] = data_[y * size_ + x];
			}
		}

		slowSwap(rotated, data_);
		delete[](rotated);
	}

	void Kernel2D::setAsLaplacian(Bool const with_diagonals)
	{
		RENGINE_ASSERT(size_ == 3);

		if (with_diagonals)
		{
			Real elements[] = {
				-1, -1, -1,
				-1, +8, -1,
				-1, -1, -1
			};

			for (int i = 0; i != 3 * 3; i++)
			{
				data_[i] = elements[i];
			}
		}
		else
		{
			Real elements[] = {
				+0, -1, +0,
				-1, +4, -1,
				+0, -1, +0
			};

			for (int i = 0; i != 3 * 3; i++)
			{
				data_[i] = elements[i];
			}
		}
	}

	void Kernel2D::setAsSobel(Orientation const& kernel_orientation)
	{
		RENGINE_ASSERT((size_ == 3) || (size_ == 5) || (size_ == 7) || (size_ == 9));

		setOrientation(kernel_orientation);

		//
		// Horizontal Sobel
		//
		if (size_ == 3)
		{
			float elements[] = {
				-1, 0, 1,
				-2, 0, 2,
				-1, 0, 1
			};

			for (int i = 0; i != 3 * 3; i++)
			{
				data_[i] = elements[i];
			}
		}
		else if (size_ == 5)
		{
			float elements[] = {
				-1, -2, 0, 2, 1,
				-2, -3, 0, 3, 2,
				-3, -4, 0, 4, 3,
				-2, -3, 0, 3, 2,
				-1, -2, 0, 2, 1
			};

			for (int i = 0; i != 5 * 5; i++)
			{
				data_[i] = elements[i];
			}
		}
		else if (size_ == 7)
		{
			float elements[] = {
				-1, -2, -3, 0, 3, 2, 1,
				-2, -3, -4, 0, 4, 3, 2,
				-3, -4, -5, 0, 5, 4, 3,
				-4, -5, -6, 0, 6, 5, 4,
				-3, -4, -5, 0, 5, 4, 3,
				-2, -3, -4, 0, 4, 3, 2,
				-1, -2, -3, 0, 3, 2, 1
			};

			for (int i = 0; i != 7 * 7; i++)
			{
				data_[i] = elements[i];
			}
		}
		else if (size_ == 9)
		{
			float elements[] = {
				-1, -2, -3, -4, 0, 4, 3, 2, 1,
				-2, -3, -4, -5, 0, 5, 4, 3, 2,
				-3, -4, -5, -6, 0, 6, 5, 4, 3,
				-4, -5, -6, -7, 0, 7, 6, 5, 4,
				-5, -6, -7, -8, 0, 8, 7, 6, 5,
				-4, -5, -6, -7, 0, 7, 6, 5, 4,
				-3, -4, -5, -6, 0, 6, 5, 4, 3,
				-2, -3, -4, -5, 0, 5, 4, 3, 2,
				-1, -2, -3, -4, 0, 4, 3, 2, 1
			};

			for (int i = 0; i != 9 * 9; i++)
			{
				data_[i] = elements[i];
			}
		}

		if (orientation() == Vertical)
		{
			rotateLeft();
		}
	}

	void Kernel2D::setAsPrewitt(Orientation const& kernel_orientation)
	{
		RENGINE_ASSERT(size_ == 3);

		setOrientation(kernel_orientation);

		float elements[] = {
			-1, 0, 1,
			-1, 0, 1,
			-1, 0, 1
		};

		for (int i = 0; i != 3 * 3; i++)
		{
			data_[i] = elements[i];
		}

		if (orientation() == Vertical)
		{
			rotateLeft();
		}
	}

	void Kernel2D::setAsScharr(Orientation const& kernel_orientation)
	{
		RENGINE_ASSERT(size_ == 3);

		setOrientation(kernel_orientation);

		float elements[] = {
			 -3, 0, +3,
			-10, 0, +10,
			 -3, 0, +3
		};

		for (int i = 0; i != 3 * 3; i++)
		{
			data_[i] = elements[i];
		}

		if (orientation() == Vertical)
		{
			rotateLeft();
		}
	}

} // namespace rengine

