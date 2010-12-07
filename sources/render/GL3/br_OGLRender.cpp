	#include "Renderer/gl/br_OGLRender.h"
#include "Renderer/gl/br_VBO.h"
#include "Renderer/br_Color.h"
#include "Renderer/br_Material.h"
#include "Renderer/gl/br_OGLVertexProgram.h"
#include "Renderer/gl/br_OGLFragmentProgram.h"

#include "OSspecific/br_OGLDevice.h"
#include "Math/br_Math_All.h"
#include "Console/br_Console.h"

//written by Alex Boreskov. Last version of 2007 year
#include "Renderer/gl/libExt.h"

namespace brUGE{

	DEFINE_SINGLETON(render::brOGLRender);

	namespace render{
		
		//------------------------------------------
		brOGLRender::brOGLRender() : running(false), OGLdevice(NULL), state_wireframe(false){

			// Регистрация консольных комманд.
			//------------------------------------------
			REGISTER_CONSOLE_METHOD("lastRenderError", _printLastError, brOGLRender, this);
			REGISTER_CONSOLE_METHOD("wireframeMode", _setWireframeMode, brOGLRender, this);
					
		}
		
		//------------------------------------------
		brOGLRender::~brOGLRender(){

		}
	
		//------------------------------------------
		BRvoid brOGLRender::init(const VideoMode *_vdMode, const os::brWinApp *_app)
		{
			// TODO: Выборос исключения при неуспешнйо инициализации рендер контекста.

			OGLdevice = new os::brOGLDevice(*_app, *_vdMode);
			if(!OGLdevice->initOGLWindow()){
				FATAL("cannot initialize OGL window.");
				return;
			}

			running = true;

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClearDepth(1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			
			// Устанавливаем проекцинные данные и размер окна.
			projection_.fov = 60.0f;
			projection_.nearDist = 0.5f;
			projection_.farDist = 1000.0f;

			screen_.width = _vdMode->width;
			screen_.height = _vdMode->height;
			
			mat4X4 proj = get_perspective_mat4X4(projection_.fov, screen_.width / screen_.height,
				projection_.nearDist, projection_.farDist);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//gluPerspective(projection_.fov, screen_.width / screen_.height,
			//	projection_.nearDist, projection_.farDist); 
			glLoadMatrixf(get_transpose(proj).pointer());
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
			//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		}

		BRvoid brOGLRender::shutDown(){

			//TODO: Продумать удаление всех ресурсов, которые менеджаться самой видое картой и как их констролировать.

			running = false;
		}
		
		BRbool brOGLRender::isRunning(){
			return running;
		}

		BRvoid brOGLRender::getRenderInfo(brRenderInfo &_rInfo){
			//TODO: дописать вывод инфоримации о рендер сисетме пользователя.
		}
		
		BRvoid brOGLRender::checkExtensions(){

			//TODO: сделать возможность запоминания этой инфы в рендере.

			initExtensions();

			//Multi texturing
			if(isExtensionSupported("GL_ARB_multitexture")){
				LOG_F("find extensions 'GL_ARB_multitexture'.");
				ConPrint("find extensions 'GL_ARB_multitexture'.");
			}else{
				WARNING_F("Extensions 'GL_ARB_multitexture' not supported.");
				ConWarning("Extensions 'GL_ARB_multitexture' not supported");
			}

			//VBO
			if(isExtensionSupported("GL_ARB_vertex_buffer_object")){
				LOG_F("find extensions 'GL_ARB_vertex_buffer_object'.");
				ConPrint("find extensions 'GL_ARB_vertex_buffer_object'.");
			}else{
				WARNING_F("Extensions 'GL_ARB_vertex_buffer_object' not supported.");
				ConWarning("Extensions 'GL_ARB_vertex_buffer_object' not supported");
			}

			// PBuffer
			if(	isExtensionSupported("WGL_ARB_pbuffer")			&& 
				isExtensionSupported("WGL_ARB_pixel_format")	&&
				isExtensionSupported("WGL_ARB_pixel_format")	&&
				isExtensionSupported("WGL_ARB_render_texture")	&&
				isExtensionSupported("GL_SGIS_generate_mipmap")		){
					LOG("find extensions for supporting PBuffer.");
					ConPrint("find extensions for supporting PBuffer.");
			}else{
				WARNING_F("PBuffer not supported.");
				ConWarning("PBuffer not supported.");
			}

			//ARB Fragment program
			if(isExtensionSupported("GL_ARB_fragment_program")){
				LOG_F("find extensions 'GL_ARB_fragment_program'.");
				ConPrint("find extensions 'GL_ARB_fragment_program'.");
			}else{
				WARNING_F("Extensions 'GL_ARB_fragment_program' not supported.");
				ConWarning("Extensions 'GL_ARB_fragment_program' not supported");
			}

			//ARB Vertex program
			if(isExtensionSupported("GL_ARB_vertex_program")){
				LOG_F("find extensions 'GL_ARB_vertex_program'.");
				ConPrint("find extensions 'GL_ARB_vertex_program'.");
			}else{
				WARNING_F("Extensions 'GL_ARB_vertex_program' not supported.");
				ConWarning("Extensions 'GL_ARB_vertex_program' not supported");
			}
		}
		
		// 
		//------------------------------------------
		void brOGLRender::setViewMatrix(const mat4X4 &mat){
			viewMatrix_ = mat;
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(mat.pointer());
		}

		//////////////////////////////////////////////////////////////////////////
		//	Rendering models
		
		BRvoid brOGLRender::renderModel_std(const brRenderEntity &_rm){
			//TODO: реализовать рисование модели с помощью методов glBegin glEnd
		}
		
		//------------------------------------------
		void brOGLRender::renderModel_vrt_arr(const brRenderEntity &_rm)
		{
			RENDER_CLEAR_ERRORS();

			brRenderMesh *tempMesh = _rm.mesh;
			glPushMatrix();
			glMultMatrixf(get_transpose(_rm.getOrientation()).pointer());
			//glMultMatrixf(_rm.getOrientation().pointer());

			if(_rm.useStdLighting && !_rm.notUseLighting){
				glEnable(GL_LIGHTING);
			}

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, sizeof(brVertex), tempMesh->vertices[0].pos.pointer());
//			glEnableClientState(GL_NORMAL_ARRAY);
//			glNormalPointer(GL_FLOAT, sizeof(brVertex), tempMesh->vertices[0].normal.pointer());

/*
			if(tempMesh->textured){
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				bindTexture(GL_TEXTURE_2D, tempMesh->material->getDiffuseMap());
				glTexCoordPointer(2, GL_FLOAT, sizeof(brVertex), tempMesh->vertices[0].texCoord.pointer());

				//использовать перезапись цвета меша, на цвет текстуры.
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}
*/			

			glDrawElements(GL_TRIANGLES, 3*tempMesh->numFaces, GL_UNSIGNED_INT, tempMesh->faces);
			//glDrawElements(GL_TRIANGLES, tempMesh->numFaces, GL_UNSIGNED_INT, tempMesh->faces);
			//RENDER_GUARD_FUNC("renderModel_vrt_arr", "glDrawElements");
/*
			if(tempMesh->textured){
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				unbindTexture(GL_TEXTURE_2D);
			}
*/

//			glDisableClientState(GL_NORMAL_ARRAY);			
			glDisableClientState(GL_VERTEX_ARRAY);			

			if(_rm.useStdLighting && !_rm.notUseLighting){
				glDisable(GL_LIGHTING);
			}

			glPopMatrix();
		}

		BRvoid brOGLRender::renderModel_vbo(const brRenderEntity &_rm){

			RENDER_CLEAR_ERRORS();

			brRenderMesh	    *mesh			= _rm.mesh;
/*
			brMaterial	*material		= _rm.mesh->material;
			brOGLFragmentProgram *fp	= material->pixelShader;
			brOGLVertexProgram	 *vp	= material->vertShader;
*/

			glPushMatrix();
			glMultMatrixf(get_transpose(_rm.getOrientation()).pointer());

			glEnableClientState(GL_VERTEX_ARRAY);
/*
			glEnableClientState(GL_NORMAL_ARRAY);	
*/

			mesh->vertexVBO->bind();

			glVertexPointer(3, GL_FLOAT, sizeof(brVertex), (void*)0);
/*
			glNormalPointer(GL_FLOAT, sizeof(brVertex), (void*)(sizeof(vec3f) + sizeof(vec2f)));
*/
			
/*
			if(mesh->textured){
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				// Тут для попиксельного нужен бамп тектура.
				bindTexture(GL_TEXTURE_2D, material->getDiffuseMap());
				activateTexture_VBO_VA(GL_TEXTURE0_ARB);
				glTexCoordPointer(2, GL_FLOAT, sizeof(brVertex), (void*)sizeof(vec3f));

				//использовать перезапись цвета меша, на цвет текстуры.
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

				if(_rm.usePPLighing && !_rm.notUseLighting){
					bindTexture(GL_TEXTURE1_ARB, GL_TEXTURE_2D, material->getBumpMap());
					activateTexture_VBO_VA(GL_TEXTURE1_ARB);
					glTexCoordPointer(3, GL_FLOAT, sizeof(brVertex), (void*)(2*sizeof(vec3f) + sizeof(vec2f)));

					bindTexture(GL_TEXTURE2_ARB, GL_TEXTURE_2D, material->getSpecularMap());
					activateTexture_VBO_VA(GL_TEXTURE2_ARB);
					glTexCoordPointer(3, GL_FLOAT, sizeof(brVertex), (void*)(3*sizeof(vec3f) + sizeof(vec2f)));

					//configure shader's
					vp->enable();
					vp->bind();
					fp->enable();
					fp->bind();
				}else if(_rm.useStdLighting && !_rm.notUseLighting){
					glEnable(GL_LIGHTING);
				}
			}
*/

			mesh->indexVBO->bind();

			//draw
			glDrawElements(GL_TRIANGLES, 3*mesh->numFaces, GL_UNSIGNED_INT, 0);
			RENDER_GUARD_FUNC("Render_VBO", "glDrawElements");
			
/*
			if(mesh->textured){
				if(_rm.usePPLighing && !_rm.notUseLighting){
					vp->disable();
					fp->disable();
					unbindTexture(GL_TEXTURE1_ARB, GL_TEXTURE_2D);
					//unbindTexture(GL_TEXTURE2_ARB, GL_TEXTURE_2D);
				}else if(_rm.useStdLighting && !_rm.notUseLighting){
					glDisable(GL_LIGHTING);
				}
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				unbindTexture(GL_TEXTURE0_ARB, GL_TEXTURE_2D);
			}
*/

			mesh->indexVBO->unbind();
			mesh->vertexVBO->unbind();

/*
			glDisableClientState(GL_NORMAL_ARRAY);	
*/
			glDisableClientState(GL_VERTEX_ARRAY);	

			glPopMatrix();
		}

		//////////////////////////////////////////////////////////////////////////
		//Low-level primitives rendering

		//////////////////////////////////////////////////////////////////////////
		// binding VBO
		BRvoid brOGLRender::bindVertexVBO(BRuint32 _id){
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, _id);
		}

		BRvoid brOGLRender::unbindVertexVBO(){
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}

		BRvoid brOGLRender::bindIndexVBO(BRuint32 _id){
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, _id);
		}

		BRvoid brOGLRender::unbindIndexVBO(){
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		//////////////////////////////////////////////////////////////////////////
		//texture unit's and binding and unbinding textures
		BRvoid brOGLRender::bindTexture(BRuint _textureUnit, BRuint _textureType, BRuint _textureId){
			glActiveTextureARB(_textureUnit);
			glEnable(_textureType);
			glBindTexture(_textureType, _textureId);
		}

		BRvoid brOGLRender::unbindTexture(BRuint _textureUnit, BRuint _textureType){
			glActiveTextureARB(_textureUnit);
			glDisable(_textureType);
		}

		BRvoid brOGLRender::bindTexture(BRuint _textureType, BRuint _textureId){
			bindTexture(GL_TEXTURE0_ARB, _textureType, _textureId);
		}

		BRvoid brOGLRender::unbindTexture(BRuint _textureType){
			unbindTexture(GL_TEXTURE0_ARB, _textureType);
		}

		// Методы активации текстур на стороне клиента, для задания текстурных координат
		// при отрисовке через VBO или Vertex Array (VA)
		BRvoid brOGLRender::activateTexture_VBO_VA(BRuint _textureUnit){
			glClientActiveTextureARB(_textureUnit);
		}

		BRvoid brOGLRender::deactivateTexture_VBO_VA(){
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
		}

		BRvoid brOGLRender::startLines(){
			glBegin(GL_LINES);
		}

		BRvoid brOGLRender::startQuads(){
			glBegin(GL_QUADS);
		}

		BRvoid brOGLRender::startTriangles(){
			glBegin(GL_TRIANGLES);
		}

		BRvoid brOGLRender::end(){
			glEnd();
		}		

		BRvoid brOGLRender::setColor3(const brColor &_clr){
			glColor3fv(_clr.toVec3().pointer());
		}

		BRvoid brOGLRender::setColor4(const brColor &_clr){
			glColor4fv(_clr.toVec4().pointer());
		}

		BRvoid brOGLRender::setColor3(const brColourF &_clr){
			glColor3fv(_clr.color);
		}

		BRvoid brOGLRender::setColor4(const brColourF &_clr){
			glColor4fv(_clr.color);
		}
		
		BRvoid brOGLRender::setVertex(const vec3f &_vrt){
			glVertex3fv(_vrt.pointer());
		}

		BRvoid brOGLRender::setNormal(const vec3f &_nrl){
			glNormal3fv(_nrl.pointer());
		}

		BRvoid brOGLRender::setTextureCoord2D(const vec2f &_txc){
			glTexCoord2fv(_txc.pointer());
		}

		BRvoid brOGLRender::setTextureCoord3D(const vec3f &_txc){
			glTexCoord3fv(_txc.pointer());
		}

		BRvoid brOGLRender::renderLine( const brUGE::math::vec3f &_start,
										const brUGE::math::vec3f &_end,
										const brUGE::render::brColourF &_color){
			glPushAttrib(GL_CURRENT_BIT);
			startLines();
			setColor3(_color);
			setVertex(_start);
			setVertex(_end);
			end();
			glPopAttrib();
		}

		BRvoid brOGLRender::renderTriangle( const brUGE::math::vec3f &_a,
											const brUGE::math::vec3f &_b,
											const brUGE::math::vec3f &_c,
											const brUGE::render::brColourF &_color){
			glPushAttrib(GL_CURRENT_BIT);
			setColor3(_color);
			startTriangles();
			setVertex(_a);
			setVertex(_b);
			setVertex(_c);
			end();
			glPopAttrib();
		}
		
		BRvoid brOGLRender::renderQuad( const vec3f &_a,
										const vec3f &_b,
										const vec3f &_c,
										const vec3f &_d,
										const brColourF &_color){
			glPushAttrib(GL_CURRENT_BIT);
			setColor3(_color);
			startQuads();
			setVertex(_a);
			setVertex(_b);
			setVertex(_c);
			setVertex(_d);
			end();
			glPopAttrib();
		}

		BRvoid brOGLRender::renderAABB(const brAABB &_aabb, const brColourF &_color){

			RENDER_CLEAR_ERRORS();

			glPushAttrib(GL_CURRENT_BIT);
			setColor3(_color);

			static uint16 indices[12][3] = 
			{	{0,2,3}, {3,1,0}, {4,5,7},
				{7,6,4}, {0,1,5}, {5,4,0},
				{1,3,7}, {7,5,1}, {3,2,6},
				{6,7,3}, {2,0,4}, {4,6,2}			
			};
			
			vec3f t_max = _aabb.getMax();
			vec3f t_min = _aabb.getMin();

			vec3f vertices[8] =
			{
				init_vec3(t_min[0], t_min[1], t_max[2]),//0
			    init_vec3(t_max[0], t_min[1], t_max[2]),//1
			    t_min,									//2
				init_vec3(t_max[0], t_min[1], t_min[2]),//3
				init_vec3(t_min[0], t_max[1], t_max[2]),//4
				t_max,									//5
				init_vec3(t_min[0], t_max[1], t_min[2]),//6
				init_vec3(t_max[0], t_max[1], t_min[2]) //7
			};
			
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, vertices);

			glDrawElements(GL_TRIANGLES, 3*12, GL_UNSIGNED_SHORT, indices);
			RENDER_GUARD_FUNC("renderAABB", "glDrawElements");

			glDisableClientState(GL_VERTEX_ARRAY);
			glPopAttrib();
		}

		BRvoid brOGLRender::renderSphere(const vec3f &_pos, const BRfloat _radius, const brColourF &_color){
			//set_state_wireframe(true);
			glPushAttrib(GL_CURRENT_BIT);
			setColor3(_color);
			GLUquadricObj *obj = gluNewQuadric();
			//gluQuadricDrawStyle(obj, GLU_FILL);
			glPushMatrix();
			glTranslatef(_pos[0], _pos[1], _pos[2]);
			gluSphere(obj, _radius, 15, 15);
			glPopMatrix();
			glPopAttrib();
			//set_state_wireframe(false);
		}

		BRvoid brOGLRender::wireframe_renderTriangle(const vec3f &_a, const vec3f &_b, const vec3f &_c, const brColourF &_color){
			set_state_wireframe(true);
			renderTriangle(_a, _b, _c, _color);
			set_state_wireframe(false);

		}

		BRvoid brOGLRender::wireframe_renderQuad(const vec3f &_a, const vec3f &_b, const vec3f &_c, const vec3f &_d, const brColourF &_color){
			set_state_wireframe(true);
			renderQuad(_a, _b, _c, _d, _color);
			set_state_wireframe(false);
		}	

		BRvoid brOGLRender::wireframe_renderAABB(const brAABB &_aabb, const brColourF &_color){
			set_state_wireframe(true);
			renderAABB(_aabb, _color);
			set_state_wireframe(false);
		}

		BRvoid brOGLRender::render_vbo(BRuint _primitiveType, const brVBO &_indices, const brVBO &_vertices,
			BRuint _stride, BRuint _numPrimitives, const brColourF &_color)
		{
			glPushAttrib(GL_CURRENT_BIT);
			setColor3(_color);

			glEnableClientState(GL_VERTEX_ARRAY);
			_vertices.bind();
			glVertexPointer(3, GL_FLOAT, _stride, (void*)0);
			_indices.bind();

			//draw
			glDrawElements(_primitiveType, _numPrimitives, GL_UNSIGNED_INT, 0);

			_vertices.unbind();
			_indices.unbind();

			glDisableClientState(GL_VERTEX_ARRAY);	
			glPopAttrib();
		}

		//////////////////////////////////////////////////////////////////////////
		//State change functions

		BRvoid brOGLRender::set_state_texturing2D(BRbool _state){
			if(state_texturing2D != _state){
				if(_state){
					glEnable(GL_TEXTURE_2D);
				}else{
					glDisable(GL_TEXTURE_2D);
				}
				state_texturing2D = _state;
			}
		}

		BRvoid brOGLRender::set_state_cullingBackFaces(BRbool _state){
			if(state_cullingBackFaces != _state){
				if(_state){
					glEnable(GL_CULL_FACE);
					glFrontFace(GL_CCW);
					glCullFace(GL_BACK);
				}else{
					glDisable(GL_CULL_FACE);
				}
				state_cullingBackFaces = _state;
			}
		}

		void brOGLRender::set_state_wireframe(bool state){
			if ( state_wireframe ){
				return;
			}

			if(state){
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}else{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//clean buffers

		BRvoid brOGLRender::setClearColor(const brColor &_color){
			vec4f temp = _color.toVec4();
			glClearColor(temp[0], temp[0], temp[0], temp[0]);
		}

		BRvoid brOGLRender::clearColorBuffer(){
			glClear(GL_COLOR_BUFFER_BIT);
		}

		BRvoid brOGLRender::clearColorAndDepthBuffers(){
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		//////////////////////////////////////////////////////////////////////////
		//debug function

		//log

		BRvoid brOGLRender::clearErrors(){
			//TODO: реализовать очистку ошибок.
		}

		// TODO: Продумать систему отчета ошибок и пофиксить всю эту гору борохла с фунция лога.
		//------------------------------------------
		void brOGLRender::printLastError(const char *module, const char *func,
			const char *file/* = 0*/, int line/* = 0*/)
		{
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				if (file)
					FATAL_F("file: '%s' line: %d [Render][%s][%s]:%s", file, line, module, func, (const char*)gluErrorString(error));
				else
					FATAL_F("[Render][%s][%s]:%s", module, func, (const char*)gluErrorString(error));
			}
		}

		//
		// Набор консольных комманд.
		//------------------------------------------------------------

		// 
		//------------------------------------------
		void brOGLRender::_printLastError(const brParamList &param){
			BRint error = glGetError();
			if(error != GL_NO_ERROR){
				ConWarning("[Render]: %s", (const char*)gluErrorString(error));
			}			
		}

		// Желательно в коде проводить проверку допустимости приведения типов.
		//------------------------------------------
		void brOGLRender::_setWireframeMode(const brParamList &param){
			if (param.size()){
				bool mode = param[0].toBoolean();
				set_state_wireframe(mode);
				state_wireframe = mode;
			}else{
				ConWarning(" * mode[bool]");
			}
		}

	}/*end namespace render*/
}/*end namespace brUGE*/