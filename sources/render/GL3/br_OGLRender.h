#ifndef _BR_OGLRENDER_H_
#define _BR_OGLRENDER_H_

#include "br_Types.h"
#include "../br_Color.h"
#include "../br_Mesh.h"
#include "../br_Vertex3d.h"
#include "../br_RenderEntity.h"
#include "Math/br_MathTypes.h"
#include "Math/br_AABoundingBox.h"
#include "Utils/br_Singleton.h"
#include "Utils/br_String.h"
#include "Console/br_Console.h"

//TODO: Никакой загрузки из ренедера не должно идти. Удалить это...
#include "Loader/br_ResourceManager.h"
#include "Loader/br_MeshManager.h"
#include "Loader/br_TextureManager.h"

//forward declaration
namespace brUGE{
	namespace os{
		class brWinApp;
		class brOGLDevice;
	}/*end namespace OS*/
}/*end namespace brUGE*/


namespace brUGE{
	namespace render{
		
		using namespace math;

		//forward declaration
		struct brColor;
		struct VideoMode;
		class brTexture;

		// Хранит информацию необходимую для построения
		// проекционной матрицы.
		//------------------------------------------
		struct brProjection{
			float	fov;
			float	nearDist;
			float	farDist;			
		};

		struct brScreenResolution{
			float width;
			float height;
		};

		struct brRenderInfo{
			const char	*renderer_string;
			const char	*vendor_string;
			const char	*version_string;
			const char	*extensions_string;
			const char	*wgl_extensions_string;

			float		glVersion;				// atof( version_string )


			int			maxTextureSize;			// queried from GL
			int			maxTextureUnits;
			int			maxTextureCoords;
			int			maxTextureImageUnits;
			float		maxTextureAnisotropy;

			int			colorBits, depthBits, stencilBits;
			bool		multitextureAvailable;
			bool		textureCompressionAvailable;
			bool		anisotropicAvailable;
			bool		textureLODBiasAvailable;
			bool		textureEnvAddAvailable;
			bool		textureEnvCombineAvailable;
			bool		registerCombinersAvailable;
			bool		cubeMapAvailable;
			bool		envDot3Available;
			bool		texture3DAvailable;
			bool		sharedTexturePaletteAvailable;
			bool		ARBVertexBufferObjectAvailable;
			bool		ARBVertexProgramAvailable;
			bool		ARBFragmentProgramAvailable;
			bool		twoSidedStencilAvailable;
			bool		textureNonPowerOfTwoAvailable;
			bool		depthBoundsTestAvailable;
		};
	
		//
		// Главный класс для отрисовки сцены, управляет низкоуровневыми функциями 
		//------------------------------------------------------------
		class brOGLRender : public Singleton<brOGLRender>{

			DECLARATE_SINGLETON(brOGLRender);

		public:
			brOGLRender();
			~brOGLRender();

			//------------------------------------------
			void init(const VideoMode *_vdMode, const brUGE::os::brWinApp *_app);

			//------------------------------------------
			void shutDown();

			//------------------------------------------
			bool isRunning();

			//------------------------------------------
			void getRenderInfo(brRenderInfo &_rInfo);

			// Проверка поддержки необходимых расширений
			//------------------------------------------
			void checkExtensions();

			//------------------------------------------
			void showSplashScreen(const brStr &_splashScreen, const brStr &_progressLine){
				splashScreen = TEXTURE_MANAGER_LOAD2D(_splashScreen);
				progressLine = TEXTURE_MANAGER_LOAD2D(_progressLine);
				showSplashScrn = true;
			}

			//------------------------------------------
			void hideSplashScreen(){
				showSplashScrn = false;
			}

			// create screen shot
			void createSreenShot(const brStr &_path = "") const;

			//------------------------------------------	
			const brProjection& projection() const { return projection_; }
			
			//------------------------------------------
			const brScreenResolution& screenResolution() const { return screen_; }

			void setViewMatrix(const mat4X4 &mat);
			const mat4X4& getViewMatrix() { return viewMatrix_; }


			//////////////////////////////////////////////////////////////////////////
			//					low-level primitives rendering						//
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			//VBO binding function
			BRvoid bindVertexVBO(BRuint32 _id);
			BRvoid unbindVertexVBO();

			BRvoid bindIndexVBO(BRuint32 _id);
			BRvoid unbindIndexVBO();

			//////////////////////////////////////////////////////////////////////////
			//texture unit's and binding and unbinding textures
			BRvoid bindTexture(BRuint _textureUnit, BRuint _textureType, BRuint _textureId);
			BRvoid unbindTexture(BRuint _textureUnit, BRuint _textureType);

			//for unit 0
			BRvoid bindTexture(BRuint _textureType, BRuint _textureId);
			BRvoid unbindTexture(BRuint _textureType);

			// Методы активации текстур на стороне клиента, для задания текстурных координат
			// при отрисовке через VBO или Vertex Array (VA)
			BRvoid activateTexture_VBO_VA(BRuint _textureUnit);
			BRvoid deactivateTexture_VBO_VA();

			//////////////////////////////////////////////////////////////////////////
			// vertex, index, texture coordinates, normal array configure
			//BRvoid bindCompositeArray(BRvoid *_vertArray, BRuint _stride);
			//BRvoid unbindCompositeArray();

			//BRvoid bindIndexArray(BRvoid *_indicesArray, BRuint _primitiveType,
			//	BRuint _numOfPrimitives, BRuint _inidicesFormat/* = GL_UNSIGNED_INT*/);

			//////////////////////////////////////////////////////////////////////////
			//standard rendering use glBegin...glEnd
			BRvoid startLines();
			BRvoid startTriangles();
			BRvoid startQuads();
			BRvoid end();

			//////////////////////////////////////////////////////////////////////////
			//set standard parameters to render
			BRvoid setColor3(const brColor &_clr);
			BRvoid setColor4(const brColor &_clr);
			BRvoid setColor3(const brColourF &_clr);
			BRvoid setColor4(const brColourF &_clr);
			BRvoid setVertex(const vec3f &_vrt);
			BRvoid setNormal(const vec3f &_nrl);
			BRvoid setTextureCoord2D(const vec2f &_txc);
			BRvoid setTextureCoord3D(const vec3f &_txc);

			//////////////////////////////////////////////////////////////////////////
			//render primitives
			//////////////////////////////////////////////////////////////////////////

			BRvoid renderLine(const vec3f &_start, const vec3f &_end, const brColourF &_color);
			BRvoid renderTriangle(const vec3f &_a, const vec3f &_b, const vec3f &_c, const brColourF &_color);
			BRvoid renderQuad(const vec3f &_a, const vec3f &_b, const vec3f &_c, const vec3f &_d, const brColourF &_color);
			BRvoid renderAABB(const brAABB &_aabb, const brColourF &_color);
			BRvoid renderSphere(const vec3f &_pos, const BRfloat _radius, const brColourF &_color);

			BRvoid wireframe_renderTriangle(const vec3f &_a, const vec3f &_b, const vec3f &_c, const brColourF &_color);
			BRvoid wireframe_renderQuad(const vec3f &_a, const vec3f &_b, const vec3f &_c, const vec3f &_d, const brColourF &_color);		
			BRvoid wireframe_renderAABB(const brAABB &_aabb, const brColourF &_color);

			// рисование примитивов из ВБО-буфферов.
			// _stride - смещение в вершинно буфере.
			// _numPrimitives - это есть (количество примитивов)*(количество вершин нужных этому примитиву)
			BRvoid render_vbo(BRuint _primitiveType, const brVBO &_indices, const brVBO &_vertices,
				BRuint _stride, BRuint _numPrimitives, const brColourF &_color);

			//////////////////////////////////////////////////////////////////////////
			//						High-level rendering							//
			//	Rendering models and bounding boxes and others 

			//render full mesh
			BRvoid renderModel_std		(const brRenderEntity &_rm);
			BRvoid renderModel_vrt_arr	(const brRenderEntity &_rm);
			BRvoid renderModel_vbo		(const brRenderEntity &_rm);

			//for capabilities
			brUGE::os::brOGLDevice *getOGLDevice(){
				return OGLdevice;
			}

			//////////////////////////////////////////////////////////////////////////
			//State change functions
			BRvoid set_state_texturing2D(BRbool _state);
			BRvoid set_state_cullingBackFaces(BRbool _state);
			BRvoid set_state_wireframe(BRbool _state);


			//////////////////////////////////////////////////////////////////////////
			//clean buffers
			BRvoid clearColorBuffer();
			BRvoid clearColorAndDepthBuffers();
			BRvoid setClearColor(const brColor &_color);

			//////////////////////////////////////////////////////////////////////////
			//debug function

			//log
			static BRvoid clearErrors();

			// TODO: Продумать систему отчета ошибок и пофиксить всю эту гору борохла с фунция лога.
			//------------------------------------------
			static void printLastError(const char *module, const char *func,
				const char *file = 0, int line = 0);

		private:
			void _printLastError(const brParamList &param);
			void _setWireframeMode(const brParamList &param);

		private:
			mat4X4					viewMatrix_;
			os::brOGLDevice			*OGLdevice;
			brProjection			projection_;	
			brScreenResolution		screen_;

			BRbool					running;	

			//OGL states
			BRbool					state_texturing2D;
			BRbool					state_cullingBackFaces;
			BRbool					state_depthTest;
			BRbool					state_lighting;
			BRbool					state_wireframe;
		public:
			//slash screen
			bool					showSplashScrn;
			brTexture				*splashScreen;
			brTexture				*progressLine;
		};

	}/*end namespace render*/
}/*end namespace brUGE*/

#define RENDER_DEBUG_MESSAGES 1

// Макросы для проверки состояния ошибки OpenGL
#if RENDER_DEBUG_MESSAGES == 1 
	#define RENDER_LOG LOG_F
	#define RENDER_WARNING WARNING_F
	#define RENDER_CLEAR_ERRORS() brOGLRender::getSingletonPtr()->clearErrors()
	#define RENDER_GUARD_FUNC(module, func) brOGLRender::getSingletonPtr()->printLastError(module, func, __FILE__, __LINE__)
#else 
	#define RENDER_LOGF
	#define RENDER_WARNING
	#define RENDER_CLEAR_ERRORS()
	#define RENDER_GUARD_FUNC(module, func)
#endif

#endif /*_BR_OGLRENDER_H_*/

/**
	* 19/01/2008
		created.
*/

