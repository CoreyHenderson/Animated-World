#include "Week9Application.h"

#include "stdafx.h"
#include "Surface.h"

void Surface::init()
{
	// Camera data
	cameraPosition = glm::vec3(65.0f, 7.5f, -45.0f);
	cameraForward = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	rightView();

	// Movement data
	speed = 0.2f;
	angleSpeed = 0.3f;

	isOnEagleView = false;
}

// Camera Eagle View Motion
void Surface::setEagleView()
{
	vec3 eaglePosition = world->getEaglePosition();		// Gets position of the eagle
	vec3 eagleRotation = world->getEagleRotation();		// Gets rotation of the eagle
	
	double clock = world->getClock();					// Gets the world clock

	// Calculates same circular position movement as the eagle, only with the clock a little less so that the camera sits just behind it.
	float radius = 13;
	float xPos = radius * (float)sin(clock - 0.4);
	float zPos = radius * (float)cos(clock - 0.4);

	// Apply camera position - X and Z are multiplied so it's centered behind the eagle (to make up for the rotation)
	cameraPosition = vec3(xPos * 1.09, eaglePosition.y + 3.0, zPos * 1.09);
	// Set camera to face upwards on it's roll
	cameraUp = vec3(0.0, 1.0, 0.0);
	// Apply rotation to the camera using eagle's rotation - Z is negated and Y is multiplied by 0.1 to lessen the effect of it's Y change from tilting the bird.
	//														 +0.4 is applied to face the camera downwards at the eagle
	cameraForward = vec3(eagleRotation.x, (-eagleRotation.y * 0.1) + 0.4f, -eagleRotation.z);
}

LRESULT CALLBACK TWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}

Surface::Surface(HINSTANCE hInst, HWND hWnd)
{
	init();

	WNDCLASSEX winClass;
	// populate window class struct
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = 0;                                     // class styles: CS_OWNDC, CS_PARENTDC, CS_CLASSDC, CS_GLOBALCLASS, ...
	winClass.lpfnWndProc = TWndProc;                  // pointer to window procedure
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = hInst;                              // owner of this class
	winClass.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SHADERWORLD));   // default icon
	winClass.hIconSm = 0;
	winClass.hCursor = LoadCursor(0, IDC_ARROW);              // default arrow cursor
	winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);   // default white brush
	winClass.lpszMenuName = 0;
	winClass.lpszClassName = L"GraphicsWindow";
	winClass.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));   // default small icon
	RegisterClassEx(&winClass);

	RECT rect;
	GetClientRect(hWnd, &rect);
	handle = CreateWindowEx(WS_EX_CLIENTEDGE, L"GraphicsWindow", L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, rect.right, rect.bottom, hWnd, NULL, hInst, NULL);

	DWORD x = GetLastError();

	hDC = GetDC(handle);

	PIXELFORMATDESCRIPTOR pfd, *ppfd;
	int pixelformat;

	ppfd = &pfd;

	ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
	ppfd->nVersion = 1;
	ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	ppfd->dwLayerMask = PFD_MAIN_PLANE;
	ppfd->iPixelType = PFD_TYPE_RGBA;
	ppfd->cColorBits = 32;
	ppfd->cDepthBits = 24;	// Was 16 bits
	ppfd->cAccumBits = 0;
	ppfd->cStencilBits = 0;

	pixelformat = ChoosePixelFormat(hDC, ppfd);
	SetPixelFormat(hDC, pixelformat, ppfd);

	HGLRC hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);

	GetClientRect(handle, &rect);
	windowWidth = rect.right - rect.left;
	windowHeight = rect.bottom - rect.top;
	resizeGL(rect.right - rect.left, rect.bottom - rect.top);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		exit(0);
	}

	initializeGL();

	ShowWindow(handle, SW_SHOWDEFAULT);
}

Surface::~Surface(void)
{
}

void Surface::resize(int width, int height)
{
	MoveWindow(handle, 0, 0, width, height, TRUE);
	windowWidth = width;
	windowHeight = height;
	resizeGL(width, height);
}

void Surface::initializeGL()
{
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	//glClearColor(0, 1, 0, 1);
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);	// Set clear colour to be a mid-grey!

	setupShadows();

	world = new World();
}

void Surface::setupShadows()
{
	shadowMapWidth = 256;
	shadowMapHeight = 256;

	glActiveTexture(GL_TEXTURE2);
	GLuint tid;
	glGenTextures(1, &tid);
	glBindTexture(GL_TEXTURE_2D, tid);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// To avoid 'Over sampling'
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glGenFramebuffers(1, &shadowTextureBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowTextureBuffer);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tid, 0);
	GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (err != GL_FRAMEBUFFER_COMPLETE)
	{
		//reportError ("Problem with shadow frame buffer", "Framebuffer not complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Surface::setShadowMapBuffer()
{
	// Render shadow map
	glBindFramebuffer(GL_FRAMEBUFFER, shadowTextureBuffer);
	GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	glViewport(0, 0, shadowMapWidth, shadowMapHeight);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Surface::restoreShadowMapBuffer(HWND hWnd)
{
	glFlush();
	// Switch back to normal rendering.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	RECT rect;
	GetClientRect(hWnd, &rect);
	glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);
}

void Surface::resizeGL(int width, int height)
{
	glViewport(0, 0, (GLint)width, (GLint)height);
}

void Surface::paintGL()
{
	//glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 view = mat4(1.0f);
	//mat4 proj = glm::frustum(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 200.0f);
	mat4 proj = glm::perspective(
		glm::radians(30.0f),			// The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
		(float)(windowWidth / windowHeight),       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
		0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
		200.0f             // Far clipping plane. Keep as little as possible.
	);
	mat4 shadow = mat4(1.0f);

	vec3 dirforward = glm::normalize(cameraForward);
	vec3 dirup = glm::normalize(cameraUp);

	vec3 dirside = glm::cross(dirup, dirforward);

	mat4 rot = mat4(dirside[0], dirside[1], dirside[2], 0.0,
					dirup[0], dirup[1], dirup[2], 0.0,
					dirforward[0], dirforward[1], dirforward[2], 0.0,
					0.0, 0.0, 0.0, 1.0);
	mat4 M = glm::translate(mat4(1.0f), cameraPosition) * rot;
	view = glm::inverse(M);

	if (isOnEagleView)
		setEagleView();

	world->setWindowSize(windowWidth, windowHeight);
	world->render(view, proj, shadow);

	SwapBuffers(hDC);
}

void Surface::strafe(float direction)
{
	if (isOnEagleView)
		isOnEagleView = false;

	vec3 dirforward = normalize(cameraForward);
	vec3 dirup = normalize(cameraUp);
	
	vec3 dirside = glm::cross(dirup, dirforward);

	cameraPosition += speed * direction * dirside;
}

void Surface::step(float direction)
{
	if (isOnEagleView)
		isOnEagleView = false;

	vec3 dirforward = normalize(cameraForward);

	cameraPosition += speed * direction * dirforward;
}

mat3 Surface::rotationVectorAngle(float ux, float uy, float uz, float angle)
{
	// http://en.wikipedia.org/wiki/Rotation_matrix
	float ct = cos(angle);
	float st = sin(angle);
	float oct = 1.0f - ct;
	float ost = 1.0f - st;
	mat3 rot = mat3(ct + ux * ux * oct, ux * uy * oct - uz * st, ux * uz * oct + uy * st,
					uy * ux * oct + uz * st, ct + uy * uy * oct, uy * uz * oct - ux * st,
					uz * ux * oct - uy * st, uz * uy * oct + ux * st, ct + uz * uz * oct);
	return rot;
}

void Surface::turnside(float direction)
{
	if (isOnEagleView)
		isOnEagleView = false;

	vec3 dirforward = normalize(cameraForward);
	vec3 dirup = normalize(cameraUp);

	float angle = direction * angleSpeed;
	mat3 R = rotationVectorAngle(dirup.x, dirup.y, dirup.z, angle);

	cameraForward = R * dirforward;
}

void Surface::turnup(float direction)
{
	if (isOnEagleView)
		isOnEagleView = false;

	vec3 dirforward = normalize(cameraForward);
	vec3 dirup = normalize(cameraUp);

	vec3 dirside = glm::cross(dirup, dirforward);

	float angle = direction * angleSpeed;
	mat3 R = rotationVectorAngle(dirside.x, dirside.y, dirside.z, angle);

	cameraForward = R * dirforward;
	cameraUp = R * dirup;
}

void Surface::goUp(float direction)
{
	if (isOnEagleView)
		isOnEagleView = false;

	cameraPosition = vec3(cameraPosition.x, cameraPosition.y + direction, cameraPosition.z);
}

void Surface::goDown(float direction)
{
	if (isOnEagleView)
		isOnEagleView = false;

	cameraPosition = vec3(cameraPosition.x, cameraPosition.y + direction, cameraPosition.z);
}

void Surface::frontView()
{
	if (isOnEagleView)
		isOnEagleView = false;

	cameraPosition = vec3(0.0f, 15.0f, -75.0f);
	cameraForward = vec3(0.0f, 0.0f, -1.0f);
	cameraUp = vec3(0.0f, 1.0f, 0.0f);
	turnup(0.2);
}

void Surface::backView()
{
	if (isOnEagleView)
		isOnEagleView = false;

	cameraPosition = vec3(0.0f, 15.0f, 75.0f);
	cameraForward = vec3(0.0f, 0.0f, 1.0f);
	cameraUp = vec3(0.0f, 1.0f, 0.0f);
	turnup(0.2);
}

void Surface::leftView()
{
	if (isOnEagleView)
		isOnEagleView = false;

	cameraPosition = vec3(75.0f, 15.0f, 0.0f);
	cameraForward = vec3(1.0f, 0.0f, 0.0f);
	cameraUp = vec3(0.0f, 1.0f, 0.0f);
	turnup(0.2);
}

void Surface::rightView()
{
	if (isOnEagleView)
		isOnEagleView = false;

	cameraPosition = vec3(-75.0f, 15.0f, 0.0f);
	cameraForward = vec3(-1.0f, 0.0f, 0.0f);
	cameraUp = vec3(0.0f, 1.0f, 0.0f);
	turnup(0.2);
}

void Surface::topView()
{
	if (isOnEagleView)
		isOnEagleView = false;

	cameraPosition = vec3(0.0f, 99.0f, 0.0f);
	cameraForward = vec3(0.0f, 0.0f, -1.0f);
	cameraUp = vec3(0.0f, 1.0f, 0.0f);
	turnup(5.2);
}

void Surface::enableEagleView()
{
	if (isOnEagleView)
		isOnEagleView = false;
	else
		isOnEagleView = true;
}

void Surface::resetOrientation()
{
	if (isOnEagleView)
		isOnEagleView = false;

	cameraUp = vec3(0.0f, 1.0f, 0.0f);
}
