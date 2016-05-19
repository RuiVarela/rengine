#include <rengine/capture/VideoCaptureDShow.h>
#include <rengine/image/Colorspace.h>
#include <rengine/thread/Synchronization.h>
#include <cmath>
#include <dshow.h>
#include <comdef.h>

#include <list>

#define RENGINE_MAX_FRAMES_BUFFERED 3

//
// compatibility Code
//
#ifndef __ISampleGrabberCB_INTERFACE_DEFINED__
#define __ISampleGrabberCB_INTERFACE_DEFINED__

EXTERN_C const IID IID_ISampleGrabberCB;

MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) = 0;
};

#endif //__ISampleGrabberCB_INTERFACE_DEFINED__

#ifndef __ISampleGrabber_INTERFACE_DEFINED__
#define __ISampleGrabber_INTERFACE_DEFINED__

EXTERN_C const IID IID_ISampleGrabber;

MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(/* [out][in] */ long *pBufferSize,/* [out] */ long *pBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(/* [retval][out] */ IMediaSample **ppSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB *pCallback, long WhichMethodToCallback) = 0;
};

#endif //__ISampleGrabber_INTERFACE_DEFINED__

EXTERN_C const CLSID CLSID_SampleGrabber;
class DECLSPEC_UUID("C1F400A0-3F08-11d3-9F0B-006008039E37") SampleGrabber;

EXTERN_C const CLSID CLSID_NullRenderer;
class DECLSPEC_UUID("C1F400A4-3F08-11d3-9F0B-006008039E37") NullRenderer;


#define CHECK_HR(hr) if (FAILED(hr)) { return; }

namespace rengine
{
	struct ImageBuffer
	{
		typedef std::list< SharedPointer<VideoCapture::Frame> > BufferedImages;

		ImageBuffer()
		{
			total_frames = 0;
			frames_dropped = 0;
		}

		~ImageBuffer()
		{
			clear();
		}

		SharedPointer<VideoCapture::Frame> pop()
		{
			ScopedLock lock(mutex);
			SharedPointer<VideoCapture::Frame> frame;

			if (!buffer.empty())
			{
				frame = buffer.front();
				buffer.pop_front();
			}

			return frame;
		}

		void push(SharedPointer<VideoCapture::Frame> frame)
		{
			ScopedLock lock(mutex);

			if (buffer.size() > RENGINE_MAX_FRAMES_BUFFERED)
			{
				frame->release();
				++frames_dropped;
			}
			else
			{
				buffer.push_back(frame);
			}

			++total_frames;
		}

		void clear()
		{
			ScopedLock lock(mutex);

			while(!buffer.empty())
			{
				buffer.front()->release();
				buffer.pop_front();
			}

			total_frames = 0;
			frames_dropped = 0;
		}

		Uint64 total_frames;
		Uint64 frames_dropped;
		BufferedImages buffer;
		Mutex mutex;
	};

	//
	// Sample Grabber
	//
	class SampleGrabber : public ISampleGrabberCB
	{
	public:
		SampleGrabber(ImageBuffer* buffer) :m_buffer(buffer) {}
		~SampleGrabber() {}	

		STDMETHODIMP_(ULONG) AddRef() { return 1; }
		STDMETHODIMP_(ULONG) Release() { return 2; }

		STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
		{
			*ppvObject = static_cast<ISampleGrabberCB*>(this);
			return S_OK;
		}

		STDMETHODIMP SampleCB(double Time, IMediaSample *pSample)
		{
			unsigned char* data = 0;
			SharedPointer<VideoCapture::Frame> frame = new VideoCapture::Frame();
			frame->size = pSample->GetActualDataLength();
			HRESULT hr = pSample->GetPointer(&data);
			frame->time_stamp = Time;

			if ((hr == S_OK) && frame->size)
			{
				frame->data = new Uchar[frame->size];
				memcpy(frame->data, data, frame->size);
			}

			if (frame->size && frame->data)
			{
				m_buffer->push(frame);
			}

			return S_OK;
		}

		STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen) { return E_NOTIMPL; }
	private:
		ImageBuffer* m_buffer;
	};

	struct VideoCaptureDShow::PrivateImplementation
	{
		static unsigned int initializations;

		PrivateImplementation()
		{
			initialize();

			graph = 0;
			video_input_filter = 0;
			capture_graph = 0;
			stream_config = 0;
			sample_grabber_filter = 0;
			sample_grabber = 0;
			grabber = 0;
			control = 0;
			renderer = 0;
		}

		~PrivateImplementation()
		{
			uninitialize();
		}

		static void initialize()
		{
			if (initializations == 0)
			{
				HRESULT hr = NULL;
				hr = CoInitialize(NULL);

				if(hr == RPC_E_CHANGED_MODE)
				{
					throw VideoCaptureException(1001, "Unable to CoInitialize");
				}
			}

			++initializations;
		}

		static void uninitialize()
		{
			--initializations;
			if (initializations == 0)
			{
				CoUninitialize();
			}
		}

		IGraphBuilder *graph;
		IBaseFilter* video_input_filter;
		ICaptureGraphBuilder2 *capture_graph;
		IAMStreamConfig *stream_config;

		IBaseFilter *sample_grabber_filter;
		ISampleGrabber* sample_grabber;
		SampleGrabber* grabber;
		ImageBuffer image_buffer;
		IMediaControl *control;
		IBaseFilter *renderer;
	};
	unsigned int VideoCaptureDShow::PrivateImplementation::initializations = 0;

	//
	// Microsoft MSDN
	//
	static HRESULT InitCaptureGraphBuilder(IGraphBuilder **ppGraph,  ICaptureGraphBuilder2 **ppBuild)
	{

		if (!ppGraph || !ppBuild)
		{
			return E_POINTER;
		}

		*ppBuild = 0;
		*ppGraph = 0;

		IGraphBuilder *pGraph = NULL;
		ICaptureGraphBuilder2 *pBuild = NULL;

		// Create the Capture Graph Builder.
		HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild );
		if (SUCCEEDED(hr))
		{
			// Create the Filter Graph Manager.
			hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
			if (SUCCEEDED(hr))
			{
				// Initialize the Capture Graph Builder.
				pBuild->SetFiltergraph(pGraph);

				// Return both interface pointers to the caller.
				*ppBuild = pBuild;
				*ppGraph = pGraph; // The caller must release both interfaces.
				return S_OK;
			}
		}

		if (pGraph)
		{
			pGraph->Release();
			pGraph = 0;
		}

		
		if (pBuild)
		{
			pBuild->Release();
			pBuild = 0;
		}
		
		return hr; // Failed
	}

	static void GetDevice(IBaseFilter** filter, VideoCapture::Device const& device)
	{
		if (device.location.empty())
		{
			return;
		}

		*filter = 0;

		ICreateDevEnum *device_enum = 0;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&device_enum));

		if (SUCCEEDED(hr))
		{
			IEnumMoniker *enum_moniker = 0;
			hr = device_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);
			if(hr == S_OK)
			{
				IMoniker *moniker = 0;
				while ( (enum_moniker->Next(1, &moniker, 0) == S_OK) && !(*filter))
				{
					IPropertyBag *property_bag;
					hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&property_bag));

					if (FAILED(hr))
					{
						moniker->Release();
						continue; 
					} 

					VARIANT variant;
					VariantInit(&variant);

					std::string location;
					if (SUCCEEDED( property_bag->Read(L"DevicePath", &variant, 0) ))
					{
						char* char_str = _com_util::ConvertBSTRToString(variant.bstrVal);
						location = char_str;
						delete[](char_str);
					}

					if (location == device.location)
					{
						hr = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)filter);

						if (FAILED(hr))
						{
							*filter = 0;
						}
					}
 
					VariantClear(&variant);
					property_bag->Release();
					property_bag = 0;

					moniker->Release();
					moniker = 0;
				}   

				device_enum->Release();
				device_enum = 0;

				enum_moniker->Release();
				enum_moniker = 0;
			}
		}
	}

	void NukeDownstream(IBaseFilter *pBF, IGraphBuilder* pGraph){
		IPin *pP, *pTo;
		ULONG u;
		IEnumPins *pins = NULL;
		PIN_INFO pininfo;
		HRESULT hr = pBF->EnumPins(&pins);
		pins->Reset();
		while (hr == NOERROR)
		{
			hr = pins->Next(1, &pP, &u);
			if (hr == S_OK && pP)
			{
				pP->ConnectedTo(&pTo);
				if (pTo)
				{
					hr = pTo->QueryPinInfo(&pininfo);
					if (hr == NOERROR)
					{
						if (pininfo.dir == PINDIR_INPUT)
						{
							NukeDownstream(pininfo.pFilter, pGraph);
							pGraph->Disconnect(pTo);
							pGraph->Disconnect(pP);
							pGraph->RemoveFilter(pininfo.pFilter);
						}
						pininfo.pFilter->Release();
						pininfo.pFilter = NULL;
					}
					pTo->Release();
				}
				pP->Release();
			}
		}
		if (pins) pins->Release();
	} 

	static void FreeMediaType(AM_MEDIA_TYPE& mt)
	{
		if (mt.cbFormat != 0)
		{
			CoTaskMemFree((PVOID)mt.pbFormat);
			mt.cbFormat = 0;
			mt.pbFormat = NULL;
		}

		if (mt.pUnk != 0)
		{
			mt.pUnk->Release();
			mt.pUnk = 0;
		}
	}

	static void FreeMediaType(AM_MEDIA_TYPE *pmt)
	{
		if (pmt != NULL)
		{
			FreeMediaType(*pmt); 
			CoTaskMemFree(pmt);
		}
	}

	//
	// Supported formats
	//
	static std::string mediaTypeToFourcc(GUID& mediatype)
	{
		std::string fourcc;

		if (mediatype == MEDIASUBTYPE_YUYV)
		{
			fourcc = "YUYV";
		}
		else if (mediatype == MEDIASUBTYPE_YUY2)
		{
			fourcc = "YUY2";
		}
		else if (mediatype == MEDIASUBTYPE_MJPG)
		{
			fourcc = "MJPG";
		}
		else if (mediatype == MEDIASUBTYPE_IJPG)
		{
			fourcc = "JPEG";
		}
		else if (mediatype == MEDIASUBTYPE_RGB24)
		{
			fourcc = "RGB3";
		}

		return fourcc;
	}

	static GUID fourccToMediaType(std::string const& fourcc)
	{
		GUID mediatype = MEDIASUBTYPE_YUY2;

		if (fourcc == "YUYV")
		{
			mediatype = MEDIASUBTYPE_YUYV;
		}
		else if (fourcc == "YUY2")
		{
			mediatype = MEDIASUBTYPE_YUY2;
		}
		else if (fourcc == "MJPG")
		{
			mediatype = MEDIASUBTYPE_MJPG;
		}
		else if (fourcc == "JPEG")
		{
			mediatype = MEDIASUBTYPE_IJPG;
		}
		else if (fourcc == "RGB3")
		{
			mediatype = MEDIASUBTYPE_RGB24;
		}

		return mediatype;
	}

	VideoCaptureDShow::VideoCaptureDShow()
		:implementation_(new PrivateImplementation()), streaming_(false)
	{

	}

	VideoCaptureDShow::VideoCaptureDShow(CaptureOptions const& capture_options)
		:implementation_(new PrivateImplementation()), streaming_(false)
	{
		try
		{
			open(capture_options);
		}
		catch (...)
		{
			close();
			delete (implementation_);
		}
	}

	VideoCaptureDShow::~VideoCaptureDShow()
	{
		close();
		delete(implementation_);
	}

	VideoCapture::Formats VideoCaptureDShow::enumerateFormats(Device const& device) const
	{
		int const frame_rate_increment = 5;

		Formats formats;

		IGraphBuilder *graph_builder = 0;
		ICaptureGraphBuilder2 *capture_graph = 0;
		IAMStreamConfig *stream_config = 0;
		IBaseFilter *video_input_filter = 0;

		GUID capture_method = PIN_CATEGORY_CAPTURE; //PIN_CATEGORY_PREVIEW

		HRESULT hr = InitCaptureGraphBuilder(&graph_builder, &capture_graph);
		if (SUCCEEDED(hr))
		{
			GetDevice(&video_input_filter, device);
			if (video_input_filter)
			{
				hr = graph_builder->AddFilter(video_input_filter, 0);
				hr = capture_graph->FindInterface(&capture_method, &MEDIATYPE_Video, video_input_filter, IID_IAMStreamConfig, (void**)&stream_config);
			}
		}

		if (stream_config)
		{
			int number_of_capabilities = 0;
			int size = 0;
			hr = stream_config->GetNumberOfCapabilities(&number_of_capabilities, &size);

			if (SUCCEEDED(hr) && size == sizeof(VIDEO_STREAM_CONFIG_CAPS))
			{
				for (int i = 0; i != number_of_capabilities; ++i)
				{
					VIDEO_STREAM_CONFIG_CAPS stream_config_caps;
					AM_MEDIA_TYPE *media_type;
					hr = stream_config->GetStreamCaps(i, &media_type, (BYTE*)&stream_config_caps);

					if (FAILED(hr))
					{
						continue;
					}

					Format format;
					format.width = stream_config_caps.MaxOutputSize.cx;
					format.height = stream_config_caps.MaxOutputSize.cy;
					format.mode = mediaTypeToFourcc(media_type->subtype);
					format.sample_size = media_type->lSampleSize;

					format.interval_numerator = 1;
					format.interval_denominator = 1;

					//
					// Build base frame rates
					//
					if (!format.mode.empty())
					{
						int min = 0;
						int max = 0;
						if (stream_config_caps.MaxFrameInterval != 0)
						{
							double denominator = 10000000.0 / stream_config_caps.MaxFrameInterval;
							min = int(floor(denominator));
						}

						if (stream_config_caps.MinFrameInterval != 0)
						{
							double denominator = 10000000.0 / stream_config_caps.MinFrameInterval;
							max = int(floor(denominator));
						}

						if (min)
						{
							format.interval_denominator = min;
							formats.push_back(format);
						}

						if (min && max)
						{
							while(min < (max - frame_rate_increment))
							{
								min += frame_rate_increment;
								format.interval_denominator = min;
								formats.push_back(format);
							}
						}

						if (max)
						{
							format.interval_denominator = max;
							formats.push_back(format);
						}

						
					}

					FreeMediaType(media_type);

	
				}
			}
		}

		//
		// TearDown
		//
		if (video_input_filter && graph_builder)
		{
			FILTER_INFO filter_info;
			memset(&filter_info, 0, sizeof(FILTER_INFO));
			video_input_filter->QueryFilterInfo(&filter_info);
			filter_info.pGraph->Release();

			graph_builder->RemoveFilter(video_input_filter);
		}

		if (video_input_filter)
		{
			video_input_filter->Release();
			video_input_filter = 0;
		}

		if (stream_config)
		{
			stream_config->Release();
			stream_config = 0;
		}

		if (graph_builder)
		{
			graph_builder->Release();
			graph_builder = 0;
		}	

		if (capture_graph)
		{
			capture_graph->Release();
			capture_graph = 0;
		}	


		return formats;
	}

	VideoCapture::Devices VideoCaptureDShow::enumerateDevices() const
	{
		Devices devices;
		ICreateDevEnum *device_enum = 0;
	
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&device_enum));

		unsigned int index = 0;
		if (SUCCEEDED(hr))
		{
			IEnumMoniker *enum_moniker = 0;
			hr = device_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);
			if(hr == S_OK)
			{
				IMoniker *moniker = 0;
				while (enum_moniker->Next(1, &moniker, 0) == S_OK)
				{
					IPropertyBag *property_bag;
					hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&property_bag));

					if (FAILED(hr))
					{
						moniker->Release();
						++index;
						continue; 
					} 

					Device device;
					device.index = index;

					VARIANT variant;
					VariantInit(&variant);
					if (SUCCEEDED( property_bag->Read(L"FriendlyName", &variant, 0) ))
					{
						char* char_str = _com_util::ConvertBSTRToString(variant.bstrVal);
						device.name = char_str;
						delete[](char_str);
					}

					if (SUCCEEDED( property_bag->Read(L"DevicePath", &variant, 0) ))
					{
						char* char_str = _com_util::ConvertBSTRToString(variant.bstrVal);
						device.location = char_str;
						delete[](char_str);
					}

					VariantClear(&variant);
					property_bag->Release();
					property_bag = 0;

					moniker->Release();
					moniker = 0;

					devices.push_back(device);
				}   

				device_enum->Release();
				device_enum = 0;

				enum_moniker->Release();
				enum_moniker = 0;
				++index;
			}
		}

		return devices;
	}

	void VideoCaptureDShow::open(CaptureOptions const& capture_options)
	{
		close();
		options_ = capture_options;

		// Choose device
		Bool matched = false;
		Devices devices = enumerateDevices();
		for (Devices::const_iterator i = devices.begin(); ((i != devices.end()) && !matched); ++i)
		{
			matched = ((i->location == options_.location) || (i->index == options_.index) );
			if (matched)
			{
				options_.set(*i);
			}
		}

    	// Choose format
		Format format = VideoCapture::matchBestFormat( enumerateFormats(options_) , options_);
		options_.set(format);

		if (!format.width || !format.height || !format.interval_denominator)
		{
			return;
		}



		GUID capture_method = PIN_CATEGORY_CAPTURE; //PIN_CATEGORY_PREVIEW
		HRESULT hr = InitCaptureGraphBuilder(&implementation_->graph,  &implementation_->capture_graph);
		CHECK_HR(hr)

		GetDevice(&implementation_->video_input_filter, options_);
		if (!implementation_->video_input_filter)
		{
			return;
		}

		hr = implementation_->graph->AddFilter(implementation_->video_input_filter, 0);
		CHECK_HR(hr)

		hr = implementation_->capture_graph->FindInterface(&capture_method, 
														  &MEDIATYPE_Video, 
														  implementation_->video_input_filter, 
														  IID_IAMStreamConfig, 
														  (void**)&implementation_->stream_config);
		CHECK_HR(hr)
	

		AM_MEDIA_TYPE* media_type;
		hr = implementation_->stream_config->GetFormat(&media_type);
		if (FAILED(hr))
		{
			FreeMediaType(media_type);
			return;
		}


		if(media_type->formattype != FORMAT_VideoInfo) 
		{
			FreeMediaType(media_type);
			return;
		}

		media_type->formattype = FORMAT_VideoInfo;
		media_type->majortype  = MEDIATYPE_Video; 
		media_type->subtype	   = fourccToMediaType(options_.mode);

		VIDEOINFOHEADER *video_info = (VIDEOINFOHEADER*)media_type->pbFormat;
		video_info->AvgTimePerFrame = REFERENCE_TIME((double(options_.interval_numerator) / double(options_.interval_denominator)) * 10000000.0);

		video_info->bmiHeader.biHeight		= options_.height;
		video_info->bmiHeader.biWidth		= options_.width;
		video_info->bmiHeader.biSizeImage	= options_.sample_size;
		media_type->lSampleSize				= options_.sample_size;


		hr = implementation_->stream_config->SetFormat(media_type);
		FreeMediaType(media_type);
		CHECK_HR(hr)


		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (void**)&implementation_->sample_grabber_filter);
		CHECK_HR(hr)

		hr = implementation_->graph->AddFilter(implementation_->sample_grabber_filter, 0);
		CHECK_HR(hr)

		hr = implementation_->sample_grabber_filter->QueryInterface(IID_ISampleGrabber, (void**)&implementation_->sample_grabber);
		CHECK_HR(hr)

		implementation_->grabber = new SampleGrabber(&implementation_->image_buffer);

		hr = implementation_->sample_grabber->SetOneShot(FALSE);
		CHECK_HR(hr)

		hr = implementation_->sample_grabber->SetBufferSamples(FALSE);
		CHECK_HR(hr)

		hr = implementation_->sample_grabber->SetCallback(implementation_->grabber, 0); 
		CHECK_HR(hr)

		hr = implementation_->graph->QueryInterface(IID_IMediaControl, (void **)&implementation_->control);
		CHECK_HR(hr)


		hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&implementation_->renderer);
		CHECK_HR(hr)
	
		hr = implementation_->graph->AddFilter(implementation_->renderer, 0);	
		CHECK_HR(hr)
	
		hr = implementation_->capture_graph->RenderStream(&capture_method, &MEDIATYPE_Video, implementation_->video_input_filter, 
																							 implementation_->sample_grabber_filter, 
																							 implementation_->renderer);	
		CHECK_HR(hr)


		enableStreaming();
	}

	void VideoCaptureDShow::close()
	{
		disableStreaming();

		if (implementation_->video_input_filter && implementation_->graph)
		{
			NukeDownstream(implementation_->video_input_filter, implementation_->graph);
		}

		if (implementation_->renderer)
		{
			implementation_->renderer->Release();
			implementation_->renderer = 0;
		}

		if (implementation_->control)
		{
			implementation_->control->Release();
			implementation_->control = 0;
		}

		if(implementation_->grabber && implementation_->sample_grabber)
		{
			implementation_->sample_grabber->SetCallback(0, 1);
		}

		if (implementation_->sample_grabber)
		{
			implementation_->sample_grabber->Release();
			implementation_->sample_grabber = 0;
		}

		if (implementation_->sample_grabber_filter)
		{
			implementation_->sample_grabber_filter->Release();
			implementation_->sample_grabber_filter = 0;
		}

		if (implementation_->grabber)
		{
			implementation_->grabber->Release();
			delete(implementation_->grabber);
			implementation_->grabber = 0;
		}

		if (implementation_->stream_config)
		{
			implementation_->stream_config->Release();
			implementation_->stream_config = 0;
		}

		if (implementation_->video_input_filter && implementation_->graph)
		{
			FILTER_INFO filter_info;
			memset(&filter_info, 0, sizeof(FILTER_INFO));
			implementation_->video_input_filter->QueryFilterInfo(&filter_info);
			filter_info.pGraph->Release();

			implementation_->graph->RemoveFilter(implementation_->video_input_filter);
		}

		if (implementation_->video_input_filter)
		{
			implementation_->video_input_filter->Release();
			implementation_->video_input_filter = 0;
		}

		if (implementation_->graph)
		{
			implementation_->graph->Release();
			implementation_->graph = 0;
		}

		if (implementation_->capture_graph)
		{
			implementation_->capture_graph->Release();
			implementation_->capture_graph = 0;
		}

		
		implementation_->image_buffer.clear();
	}

	Bool VideoCaptureDShow::ready() const
	{
		return streaming_;
	}

	void VideoCaptureDShow::enableStreaming()
	{
		HRESULT hr = S_OK;
		if (implementation_->control)
		{
			hr = implementation_->control->Run();
		}

		if (SUCCEEDED(hr))
		{
			streaming_ = true;
		}
	}

	void VideoCaptureDShow::disableStreaming()
	{
		HRESULT hr = 0;
		if(implementation_->control)
		{
			hr = implementation_->control->Stop();
		}

		streaming_ = false; 
	}

	SharedPointer<VideoCapture::Frame> VideoCaptureDShow::grab(FrameOptions const& options, SharedPointer<Frame> const& frame)
	{
		if (!ready())
		{
			if (frame)
			{
				frame->release();
			}
			return 0;
		}

		Uint const frame_size = options_.width * options_.height * 3;
		SharedPointer<Frame> camera_frame = implementation_->image_buffer.pop();

		if (!camera_frame || !camera_frame->data || !camera_frame->size)
		{
			if (frame)
			{
				frame->release();
			}
			return 0;
		}

		if (((options_.mode == "RGB3") || (options_.mode == "BGR3")) && (camera_frame->size != frame_size))
		{
			if (frame)
			{
				frame->release();
			}

			camera_frame->release();
			throw VideoCaptureException(1002, "Camera sent bad data");
		}

		SharedPointer<Frame> output_frame;

		if ( (options_.mode == "YUYV") || (options_.mode == "YUY2") || 
			 (options_.mode == "MJPG") || (options_.mode == "JPEG") )
		{
			output_frame = new Frame();

			if (frame && (frame->size == frame_size))
			{
				output_frame = frame;
			}
			else if (frame && frame->data && frame->size)
			{
				frame->release();
			}
		}
		
	
		if (options_.mode == "RGB3")
		{
			if (frame)
			{
				frame->release();
			}
			output_frame = camera_frame;
			convertBGR8_RGB8(output_frame->data, options_.width, options_.height);
		}
		else if (options_.mode == "BGR3")
		{
			if (frame)
			{
				frame->release();
			}
			output_frame = camera_frame;
		}
		else if ( (options_.mode == "YUYV") || (options_.mode == "YUY2") )
		{
			if (!output_frame->data)
			{
				output_frame->data = new Uchar[frame_size];
				output_frame->size = frame_size;
			}
			//memset(output_frame->data, 127, frame_size);
			convertYUYV_RGB8(camera_frame->data, output_frame->data, options_.width, options_.height);
			camera_frame->release();
		}
		else if ( (options_.mode == "MJPG") || (options_.mode == "JPEG") )
		{
			output_frame->release();
			output_frame->size = frame_size;
			output_frame->data = convertJPEG_RGB8(camera_frame->data, camera_frame->size, output_frame->size);

			camera_frame->release();
		}
		else
		{
			if (frame)
			{
				frame->release();
			}

			if (camera_frame)
			{
				camera_frame->release();
			}
		}






		return output_frame;
	}

} //namespace rengine
