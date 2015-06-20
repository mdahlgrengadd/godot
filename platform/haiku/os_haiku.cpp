#include "servers/visual/visual_server_raster.h"
#include "servers/visual/visual_server_wrap_mt.h"
#include "drivers/gles2/rasterizer_gles2.h"
#include "servers/physics/physics_server_sw.h"
//#include "servers/physics_2d/physics_2d_server_wrap_mt.h"
#include "main/main.h"

#include "os_haiku.h"


OS_Haiku::OS_Haiku() {
	AudioDriverManagerSW::add_driver(&driver_dummy);
};

void OS_Haiku::run() {
	if (!main_loop) {
		return;
	}

	main_loop->init();
	window->Show();
	window->StartMessageRunner();
	app->Run();
	window->StopMessageRunner();

	delete app;
	main_loop->finish();
}

String OS_Haiku::get_name() {
	return "Haiku";
}

int OS_Haiku::get_video_driver_count() const {
	return 1;
}

const char* OS_Haiku::get_video_driver_name(int p_driver) const {
	return "GLES2";
}

OS::VideoMode OS_Haiku::get_default_video_mode() const {
	return OS::VideoMode(800, 600, false);
}

void OS_Haiku::initialize(const VideoMode& p_desired, int p_video_driver, int p_audio_driver) {
	main_loop = NULL;
	current_video_mode = p_desired;

	app = new HaikuApplication();

	BRect frame;
	frame.Set(50, 50, 50 + current_video_mode.width - 1, 50 + current_video_mode.height - 1);

	window = new HaikuDirectWindow(frame);

#if defined(OPENGL_ENABLED) || defined(LEGACYGL_ENABLED)
	context_gl = memnew(ContextGL_Haiku(window));
	context_gl->initialize();

	rasterizer = memnew(RasterizerGLES2);
#endif

	visual_server = memnew(VisualServerRaster(rasterizer));

	ERR_FAIL_COND(!visual_server);

	// TODO: enable multithreaded VS
	//if (get_render_thread_mode() != RENDER_THREAD_UNSAFE) {
	//	visual_server = memnew(VisualServerWrapMT(visual_server, get_render_thread_mode() == RENDER_SEPARATE_THREAD));
	//}

	visual_server->init();

	physics_server = memnew(PhysicsServerSW);
	physics_server->init();
	physics_2d_server = memnew(Physics2DServerSW);
	// TODO: enable multithreaded PS
	//physics_2d_server = Physics2DServerWrapMT::init_server<Physics2DServerSW>();
	physics_2d_server->init();

	AudioDriverManagerSW::get_driver(p_audio_driver)->set_singleton();

	if (AudioDriverManagerSW::get_driver(p_audio_driver)->init() != OK) {
		ERR_PRINT("Initializing audio failed.");
	}

	sample_manager = memnew(SampleManagerMallocSW);
	audio_server = memnew(AudioServerSW(sample_manager));
	audio_server->init();

	spatial_sound_server = memnew(SpatialSoundServerSW);
	spatial_sound_server->init();
	spatial_sound_2d_server = memnew(SpatialSound2DServerSW);
	spatial_sound_2d_server->init();

	input = memnew(InputDefault);
	window->SetInput(input);
}

void OS_Haiku::finalize() {
	if (main_loop) {
		memdelete(main_loop);
	}

	main_loop = NULL;

	spatial_sound_server->finish();
	memdelete(spatial_sound_server);

	spatial_sound_2d_server->finish();
	memdelete(spatial_sound_2d_server);

	audio_server->finish();
	memdelete(audio_server);
	memdelete(sample_manager);

	visual_server->finish();
	memdelete(visual_server);
	memdelete(rasterizer);

	physics_server->finish();
	memdelete(physics_server);

	physics_2d_server->finish();
	memdelete(physics_2d_server);

	memdelete(input);

#if defined(OPENGL_ENABLED) || defined(LEGACYGL_ENABLED)
	memdelete(context_gl);
#endif
}

void OS_Haiku::set_main_loop(MainLoop* p_main_loop) {
	main_loop = p_main_loop;
	input->set_main_loop(p_main_loop);
	window->SetMainLoop(p_main_loop);
}

MainLoop* OS_Haiku::get_main_loop() const {
	return main_loop;
}

void OS_Haiku::delete_main_loop() {
	if (main_loop) {
		memdelete(main_loop);
	}

	main_loop = NULL;
	window->SetMainLoop(NULL);
}

void OS_Haiku::release_rendering_thread() {
	context_gl->release_current();
}

void OS_Haiku::make_rendering_thread() {
	context_gl->make_current();
}

bool OS_Haiku::can_draw() const {
	// TODO: implement
	return true;
}

void OS_Haiku::swap_buffers() {
	context_gl->swap_buffers();
}

Point2 OS_Haiku::get_mouse_pos() const {
	return window->GetLastMousePosition();
}

int OS_Haiku::get_mouse_button_state() const {
	return window->GetLastButtonMask();
}

void OS_Haiku::set_cursor_shape(CursorShape p_shape) {
	//ERR_PRINT("set_cursor_shape() NOT IMPLEMENTED");
}

void OS_Haiku::set_window_title(const String& p_title) {
	window->SetTitle(p_title.utf8().get_data());
}

Size2 OS_Haiku::get_window_size() const {
	ERR_PRINT("get_window_size() NOT IMPLEMENTED");
}

void OS_Haiku::set_video_mode(const VideoMode& p_video_mode, int p_screen) {
	ERR_PRINT("set_video_mode() NOT IMPLEMENTED");
}

OS::VideoMode OS_Haiku::get_video_mode(int p_screen) const {
	return current_video_mode;
}

void OS_Haiku::get_fullscreen_mode_list(List<VideoMode> *p_list, int p_screen) const {
	ERR_PRINT("get_fullscreen_mode_list() NOT IMPLEMENTED");
}

String OS_Haiku::get_executable_path() const {
	return OS::get_executable_path();
}
