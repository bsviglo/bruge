#include "Renderer/GL/br_OGLPBuffer.h"
#include "Utils/br_AttrList.h"

namespace brUGE{
	namespace render{

		brOGLPBuffer::brOGLPBuffer( int		_width,
									int		_height,
									int		_mode,
									bool	_sharedMode): width(_width),
														   height(_height),
														   mode(_mode),
														   sharedMode(_sharedMode),
														   currentDC(NULL),
														   currentGLRC(NULL),
														   savedDC(NULL),
														   savedGLRC(NULL),
														   idTexture(0),
														   pBuffer(NULL){

		}


		bool brOGLPBuffer::create(){
			HDC	  sDC		= wglGetCurrentDC();
			HGLRC sGLRC		= wglGetCurrentContext();
			
			//check of initialization extensions
			if(wglChoosePixelFormatARB == NULL || wglCreatePbufferARB  == NULL ||
			   wglGetPbufferDCARB      == NULL || wglQueryPbufferARB   == NULL ||
			   wglReleasePbufferDCARB  == NULL || wglDestroyPbufferARB == NULL){
				return false;
			}

			brAttrList<int>		intAttrs;
			brAttrList<float>	floatAttrs;
			int format;

			intAttrs.pushEnd(WGL_SUPPORT_OPENGL_ARB,  GL_TRUE);
			intAttrs.pushEnd(WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE);
			intAttrs.pushEnd(WGL_PIXEL_TYPE_ARB,      WGL_TYPE_RGBA_ARB);
			intAttrs.pushEnd(WGL_RED_BITS_ARB,        8);
			intAttrs.pushEnd(WGL_GREEN_BITS_ARB,      8);
			intAttrs.pushEnd(WGL_BLUE_BITS_ARB,       8);
			intAttrs.pushEnd(WGL_DOUBLE_BUFFER_ARB, (mode & PBM_DOUBLE ? GL_TRUE : GL_FALSE));

			if(mode & PBM_ALPHA)
				intAttrs.pushEnd(WGL_ALPHA_BITS_ARB, 8);

			if(mode & PBM_DEPTH)
				intAttrs.pushEnd(WGL_DEPTH_BITS_ARB, 24);

			if(mode & PBM_STENSIL)
				intAttrs.pushEnd(WGL_STENCIL_BITS_ARB, 8);

			if(mode & PBM_ACCUM)
				intAttrs.pushEnd(WGL_ACCUM_BITS_ARB, 32);

			if((mode & PBM_TEXTURE_1D) || (mode & PBM_TEXTURE_2D) || (mode & PBM_TEXTURE_CUBEMAP))
				if(mode & PBM_ALPHA)
					intAttrs.pushEnd(WGL_BIND_TO_TEXTURE_RGBA_ARB, GL_TRUE);
				else
					intAttrs.pushEnd(WGL_BIND_TO_TEXTURE_RGB_ARB, GL_TRUE);

			int			pixelFormats[MAX_PBUFFER_FORMATS];
			unsigned	numFormats = 0;

			if(!wglChoosePixelFormatARB(sDC, intAttrs.getAttrList(), floatAttrs.getAttrList(),
					MAX_PBUFFER_FORMATS, pixelFormats, &numFormats))
				return false;

			if(numFormats < 1)
				return false;

			format = pixelFormats [0];

			brAttrList<int>	props;

			if((mode & PBM_TEXTURE_1D) || (mode & PBM_TEXTURE_2D))
				if(mode & PBM_ALPHA)
					props.pushEnd(WGL_TEXTURE_FORMAT_ARB,  WGL_TEXTURE_RGBA_ARB);
				else
					props.pushEnd(WGL_TEXTURE_FORMAT_ARB,  WGL_TEXTURE_RGB_ARB);

			if(mode & PBM_TEXTURE_1D)
				props.pushEnd(WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_1D_ARB);
			else if(mode & PBM_TEXTURE_2D)
					props.pushEnd(WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB);
				 else if(mode & PBM_TEXTURE_CUBEMAP)
					props.pushEnd(WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_CUBE_MAP_ARB);

			if(mode & PBM_TEXTURE_MIPMAP)
				props.pushEnd(WGL_MIPMAP_TEXTURE_ARB, GL_TRUE);

			pBuffer = wglCreatePbufferARB(sDC, format, width, height, props.getAttrList());

			if(pBuffer == NULL)
				return false;

			currentDC = wglGetPbufferDCARB(pBuffer);

			if(currentDC == NULL)
				return false;

			currentGLRC = wglCreateContext(currentDC);

			if(sharedMode)
				wglShareLists(sGLRC, currentGLRC);

			// get real size of p-buffer
			wglQueryPbufferARB(pBuffer, WGL_PBUFFER_WIDTH_ARB,  &width);
			wglQueryPbufferARB(pBuffer, WGL_PBUFFER_HEIGHT_ARB, &height);

			// now create associated texture
			if((mode & PBM_TEXTURE_2D) == 0)
				return true;

			glGenTextures(1, &idTexture);
			glBindTexture(GL_TEXTURE_2D, idTexture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// set 1-byte alignment
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);								

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			if(mode & PBM_TEXTURE_MIPMAP){
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

			return true;
		}

		bool brOGLPBuffer::destroy(){
			if(pBuffer != NULL){
				wglDeleteContext(currentGLRC);
				wglReleasePbufferDCARB(pBuffer,currentDC);
				wglDestroyPbufferARB(pBuffer);
			}
			if(idTexture){
				glDeleteTextures(1, &idTexture);
			}
			return true;
		}

		bool brOGLPBuffer::makeCurrent(){
			savedDC		= wglGetCurrentDC();
			savedGLRC	= wglGetCurrentContext();
			return wglMakeCurrent(currentDC, currentGLRC) != 0;
		}

		bool brOGLPBuffer::restoreCurrent(){
			if(!savedDC || !savedGLRC){
				return false;
			}
			//explicitly convert int to boolean
			bool result = wglMakeCurrent(savedDC, savedGLRC) & 0x01;
			savedDC		= NULL; 
			savedGLRC	= NULL;
			return result;
		}

		bool brOGLPBuffer::isLost(){
			int result = 0;
			wglQueryPbufferARB(pBuffer, WGL_PBUFFER_LOST_ARB, &result);
			//explicitly convert int to boolean
			return result & 0x01;
		}

		bool brOGLPBuffer::bind2Texture(int _buf){	
			glBindTexture(GL_TEXTURE_2D, idTexture);
			return wglBindTexImageARB(pBuffer, _buf) != FALSE;
		}

		bool brOGLPBuffer::unbindFromTexture(int _buf){
			glBindTexture(GL_TEXTURE_2D, idTexture);
			bool result = wglReleaseTexImageARB(pBuffer, _buf) != FALSE;
			glBindTexture(GL_TEXTURE_2D, 0);
			return result;
		}

		bool brOGLPBuffer::setCubeMapSide(int _side){
			if ( (mode & PBM_TEXTURE_CUBEMAP) == 0 )
				return false;
			glFlush ();
			return setAttr(WGL_CUBE_MAP_FACE_ARB, _side);
		}

		bool brOGLPBuffer::setAttr(int _attr, int _value){
			brAttrList<int> attrList;
			attrList.pushEnd(_attr, _value);
			return wglSetPbufferAttribARB(pBuffer, attrList.getAttrList()) != GL_FALSE;	
		}

		void brOGLPBuffer::checkError(){
			DWORD err = GetLastError();

			switch ( err )
			{
			case ERROR_INVALID_PIXEL_FORMAT:
				FATAL("pBuffer: ERROR_INVALID_PIXEL_FORMAT");
				break;

			case ERROR_NO_SYSTEM_RESOURCES:
				FATAL("pBuffer: ERROR_NO_SYSTEM_RESOURCES");
				break;

			case ERROR_INVALID_DATA:
				FATAL("pBuffer: ERROR_INVALID_DATA");
				break;

			case ERROR_INVALID_WINDOW_HANDLE:
				FATAL("pBuffer: ERROR_INVALID_WINDOW_HANDLE");
				break;

			case ERROR_RESOURCE_TYPE_NOT_FOUND:
				FATAL("pBuffer: ERROR_RESOURCE_TYPE_NOT_FOUND");
				break;

			case ERROR_SUCCESS:
				// no error
				break;

			default:
				LPVOID msgBuf;

				FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					err,
					MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
					(LPTSTR) &msgBuf,
					0,
					NULL );

				fprintf  ( stderr, "Error %d: %s\n", err, msgBuf );
				//FATAL("pBuffer: Error " + Rstring((Rint)err) + " " + Rstring((Rint)msgBuf));
				LocalFree( msgBuf );
				break;
			}

			SetLastError ( 0 );
		}

	}/*end namespace render*/
}/*end namespace brUGE*/