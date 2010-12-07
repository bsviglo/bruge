#ifndef _BR_OGLPBUFFER_H_
#define _BR_OGLPBUFFER_H_

#include "br_Types.h"
#include "Renderer/gl/libext.h"

namespace brUGE{
	namespace render{

		#define MAX_PBUFFER_FORMATS 20

		class brOGLPBuffer{
		public:
			/**
			 *	Pbuffer creating mode	
			 */
			enum brPBufferMode{
				PBM_ALPHA		    = 1,
				PBM_DEPTH           = 2,
				PBM_STENSIL         = 4,
				PBM_ACCUM           = 8,
				PBM_DOUBLE			= 16,
				PBM_TEXTURE_1D      = 32,
				PBM_TEXTURE_2D      = 64,
				PBM_TEXTURE_CUBEMAP = 128,
				PBM_TEXTURE_MIPMAP  = 256
			};
		    
			brOGLPBuffer(int	_width,
						 int	_height,
						 int	_mode = PBM_ALPHA | PBM_DEPTH | PBM_STENSIL,
						 bool	_sharedMode = true);

			~brOGLPBuffer();

			int getWidth() const{
				return width;
			}

			int getHeight() const{
				return height;
			}
			
			/**
			 *	porting textures and object list
			 *  from current context to pBuffer context
			 */
			bool isSharedMode(){
				return sharedMode;
			}

			/**
			 *	Retrieve texture
			 */
			unsigned getTexture(){
				return idTexture;
			}
			
			/**
			 *	Create PBuffer, this method call after initialization object
			 */
			bool create();

			/**
			 *	Destroy pBuffer
			 */
			bool destroy();

			/**
			 *	Make Pbuffer a target of drawing
			 */
			bool makeCurrent();

			/**
			 *	Restore mainframe as a target of drawing
			 */
			bool restoreCurrent();

			/**
			 *	Check of existing pBuffer
			 */
			bool isLost();

			/**
			 *	Bind buffer @_buf of pBuffer to texture 
			 */
			bool bind2Texture(int _buf = WGL_FRONT_LEFT_ARB);

			/**
			 *	Unbind buffer @_buf of pBuffer from texture
			 */
			bool unbindFromTexture(int _buf = WGL_FRONT_LEFT_ARB);
			
			/**
			 *	Set side of cubeMmap texture, into direct drawing.
			 */
			bool setCubeMapSide(int _side);
			
			/**
			 *	Set pBuffer attributes
			 */
			bool setAttr(int _attr, int _value);

			/**
			 *	Log errors
			 */
			void checkError();

		protected:
			HDC			currentDC;
			HGLRC		currentGLRC;
			HDC	 		savedDC;
			HGLRC		savedGLRC;	
			HPBUFFERARB	pBuffer;
			int			width;
			int			height;
			unsigned	idTexture;
			bool		sharedMode;
			int			mode;
		};

	}/*end namespace render*/
}/*end namespace brUGE*/

#endif /*_BR_OGLPBUFFER_H_*/ 

/**
	* 14/10/2007
		created
**/