#include "in_scene.h"
#include "mini_xfileLoader.h"
#include "mini_effectLoader.h"
#include "mini_exceptions.h"
#include <sstream>

using namespace std;
using namespace mini;
using namespace mini::utils;
using namespace DirectX;

bool INScene::ProcessMessage(WindowMessage& msg)
{
	/*Process windows messages here*/
	/*if message was processed, return true and set value to msg.result*/

	//HandleMouseChange(msg);

	return false;
}

void INScene::InitializeInput()
{
	/*Initialize Direct Input resources here*/
	InitializeEnvironment();

	HINSTANCE hInst = getHandle();
	if (FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&di), nullptr)))
		return;

	if (FAILED(di->CreateDevice(GUID_SysKeyboard, &keyboard, nullptr))
		|| FAILED(keyboard->SetDataFormat(&c_dfDIKeyboard))
		|| FAILED(keyboard->Acquire()))
		throw new exception("Cannot initialize keyboard");

	if (FAILED(di->CreateDevice(GUID_SysMouse, &mouse, nullptr))
		|| FAILED(mouse->SetDataFormat(&c_dfDIMouse))
		|| FAILED(mouse->Acquire()))
		throw new exception("Cannot initialize mouse");

	m_capabilities.dwSize = sizeof(DIDEVCAPS);
	if (FAILED(di->CreateDevice(GUID_Joystick, &joystick, nullptr))
		|| FAILED(joystick->SetDataFormat(&c_dfDIJoystick2))
		// Determine how many axis the joystick has (so we don't error out setting
		// properties for unavailable axis)
		|| FAILED(joystick->GetCapabilities(&m_capabilities))
		|| FAILED(joystick->Acquire()))
		throw new exception("Cannot initialize joystick");

	//InitializeJoystickRange();
}

void INScene::InitializeEnvironment()
{
	m_isMouseDown = false;
	m_isReturnDown = false;
	m_lastMousePosition = POINT();

	m_lastPressedButton = -1;
	m_timeLastPressElapsed = 0;

	m_showControlers = false;
	m_renderButtons = false;
	m_chosenControler = Keyboard;
	m_highlitedIndex = 0;

	m_buttons[Up] = DIK_UP;
	m_buttons[Down] = DIK_DOWN;
	m_buttons[Left] = DIK_LEFT;
	m_buttons[Right] = DIK_RIGHT;

	m_buttons[Return] = DIK_RETURN;
	m_buttons[Menu] = DIK_ESCAPE;

	m_buttonsStrings[Up] = "Up";
	m_buttonsStrings[Down] = "Down";
	m_buttonsStrings[Left] = "Left";
	m_buttonsStrings[Right] = "Right";

	m_buttonsStrings[Return] = "Open the door";
	m_buttonsStrings[Menu] = "Menu";

	m_buttonsStrings[UpRotate] = "UpRotate";
	m_buttonsStrings[DownRotate] = "DownRotate";
	m_buttonsStrings[LeftRotate] = "LeftRotate";
	m_buttonsStrings[RightRotate] = "RightRotate";
}

void INScene::InitializeJoystickRange()
{
	DIPROPRANGE diprg;
	diprg.diph.dwSize = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwHow = DIPH_BYID;
	diprg.diph.dwObj = m_capabilities.dwDevType;
	diprg.lMin = -1000;
	diprg.lMax = +1000;

	if (FAILED(joystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
		throw new exception("Cannot initialize joystick");
}

void INScene::Shutdown()
{
	/*Release Direct Input resources here*/

	m_font.reset();
	m_fontFactory.reset();
	m_sampler.reset();
	m_texturedEffect.reset();
	m_cbProj.reset();
	m_cbView.reset();
	m_cbModel.reset();
	m_sceneGraph.reset();
	DxApplication::Shutdown();

	mouse->Unacquire();
	keyboard->Unacquire();
	if (joystick)
		joystick->Unacquire();

	di->Release();
}

void INScene::Update(float dt)
{
	/*proccess Direct Input here*/

	switch (m_chosenControler)
	{
	case Keyboard:
		HandleMouseChangeDI();
		HandleKeyboardChangeDI(dt);

		//HandleKeyboardChange(dt);
		break;
	case Joystick:
		HandleJoystickChangeDI(dt);
		break;
	}

	m_counter.NextFrame(dt);
	UpdateDoor(dt);
}

void INScene::HandleMouseChange(WindowMessage& msg)
{
	switch (msg.message)
	{
		// Left mouse button down
	case WM_LBUTTONDOWN:
		m_isMouseDown = true;
		GetCursorPos(&m_lastMousePosition);
		break;
		// Mouse move
	case WM_MOUSEMOVE:
		if (!m_isMouseDown) break;
		POINT mousePosition;
		GetCursorPos(&mousePosition);
		m_camera.Rotate((m_lastMousePosition.y - mousePosition.y) * 0.002 * XM_PIDIV4, (m_lastMousePosition.x - mousePosition.x) * 0.002 * XM_PIDIV4);
		m_lastMousePosition = mousePosition;
		break;
	case WM_LBUTTONUP:
		m_isMouseDown = false;
		break;
	}
}

void INScene::HandleMouseChangeDI()
{
	DIMOUSESTATE state;
	if (GetDeviceState(mouse, sizeof(DIMOUSESTATE), &state) && state.rgbButtons[0])
		m_camera.Rotate((state.lY) * 0.002 * XM_PIDIV4, (state.lX) * 0.002 * XM_PIDIV4);
}

void INScene::HandleKeyboardChange(float dt)
{
	if (GetKeyState(VK_UP) & 0x80)
		MoveCharacter(0, dt);
	else if (GetKeyState(VK_DOWN) & 0x80)
		MoveCharacter(0, -dt);

	if (GetKeyState(VK_RIGHT) & 0x80)
		MoveCharacter(dt, 0);
	else if (GetKeyState(VK_LEFT) & 0x80)
		MoveCharacter(-dt, 0);

	if (GetKeyState(VK_RETURN) & 0x80 && DistanceToDoor() < 1.0f && FacingDoor() && !m_isReturnDown)
	{
		ToggleDoor();
		m_isReturnDown = true;
	}
}

void INScene::HandleKeyboardChangeDI(float dt)
{
	m_timeLastPressElapsed += dt;
	if (m_timeLastPressElapsed > 0.5)
		SetLastPressedButton(-1);

	BYTE keyboardState[256];
	if (GetDeviceState(keyboard, sizeof(BYTE)* 256, &keyboardState))
	{
		if (m_showControlers)
		{
			ChooseControler(keyboardState);
			return;
		}
		else if (m_renderButtons)
		{
			ChooseButton(keyboardState);
			return;
		}
		if (keyboardState[m_buttons[Menu]] && !(m_lastPressedButton == Menu))
		{
			m_showControlers = !m_showControlers;
			SetLastPressedButton(Menu);
		}

		if (keyboardState[m_buttons[Up]])
			MoveCharacter(0, dt);
		else if (keyboardState[m_buttons[Down]])
			MoveCharacter(0, -dt);

		if (keyboardState[m_buttons[Left]])
			MoveCharacter(-dt, 0);
		else if (keyboardState[m_buttons[Right]])
			MoveCharacter(dt, 0);

		if (keyboardState[m_buttons[Return]] && DistanceToDoor() < 1.0f && FacingDoor() && !m_isReturnDown)
		{
			ToggleDoor();
			m_isReturnDown = true;
		}
	}
}

void INScene::HandleJoystickChangeDI(float dt)
{
	m_timeLastPressElapsed += dt;
	if (m_timeLastPressElapsed > 0.5)
		SetLastPressedButton(-1);

	DIJOYSTATE2 state;
	if (GetDeviceState(joystick, sizeof(DIJOYSTATE2), &state))
	{
		if (m_showControlers)
		{
			ChooseControlerJoystick(state);
			return;
		}
		if (m_renderButtons)
		{
			ChooseButtonJoystick(state);
			return;
		}

		if (CheckJoystickState(state, Menu) && !(m_lastPressedButton == Menu))
		{
			m_showControlers = !m_showControlers;
			m_highlitedIndex = 0;
			SetLastPressedButton(Menu);
		}

		if (CheckJoystickState(state, Up))
			MoveCharacter(0, dt);
		else if (CheckJoystickState(state, Down))
			MoveCharacter(0, -dt);

		if (CheckJoystickState(state, Left))
			MoveCharacter(-dt, 0);
		else if (CheckJoystickState(state, Right))
			MoveCharacter(dt, 0);

		if (CheckJoystickState(state, Return) && DistanceToDoor() < 1.0f && FacingDoor() && !m_isReturnDown)
		{
			ToggleDoor();
			m_isReturnDown = true;
		}

		if (CheckJoystickState(state, UpRotate))
			m_camera.Rotate(-0.002 * XM_PIDIV4, 0);
		else if (CheckJoystickState(state, DownRotate))
			m_camera.Rotate(0.002 * XM_PIDIV4, 0);
		if (CheckJoystickState(state, RightRotate))
			m_camera.Rotate(0, 0.002 * XM_PIDIV4);
		else if (CheckJoystickState(state, LeftRotate))
			m_camera.Rotate(0, -0.002 * XM_PIDIV4);
	}
}

bool INScene::CheckJoystickState(DIJOYSTATE2 state, int value)
{
	if (m_buttons[value] >= 0 && state.rgbButtons[m_buttons[value]]
	|| m_buttons[value] == XAxisDown && state.lX - MaxAxisRange > IgnoreRange
		|| m_buttons[value] == YAxisDown && state.lY - MaxAxisRange > IgnoreRange
		|| m_buttons[value] == ZAxisDown && state.lZ - MaxAxisRange > IgnoreRange
		|| m_buttons[value] == XAxisUp && state.lX - MaxAxisRange < -IgnoreRange
		|| m_buttons[value] == YAxisUp && state.lY - MaxAxisRange < -IgnoreRange
		|| m_buttons[value] == ZAxisUp && state.lZ - MaxAxisRange < -IgnoreRange
		|| m_buttons[value] == XAxisRotateLeft && state.lRx - MaxAxisRange < -IgnoreRange
		|| m_buttons[value] == XAxisRotateRight && state.lRx - MaxAxisRange > IgnoreRange
		|| m_buttons[value] == YAxisRotateLeft && state.lRy - MaxAxisRange < -IgnoreRange
		|| m_buttons[value] == YAxisRotateRight && state.lRy - MaxAxisRange > IgnoreRange
		|| m_buttons[value] == ZAxisRotateLeft && state.lRz - MaxAxisRange < -IgnoreRange
		|| m_buttons[value] == ZAxisRotateRight && state.lRz - MaxAxisRange > IgnoreRange
		|| m_buttons[value] == SliderUp && state.rglSlider[0] - MaxAxisRange < -IgnoreRange
		|| m_buttons[value] == SliderDown && state.rglSlider[0] - MaxAxisRange > IgnoreRange
		|| m_buttons[value] == Slider1Up && state.rglSlider[1] - MaxAxisRange < -IgnoreRange
		|| m_buttons[value] == Slider1Down && state.rglSlider[1] - MaxAxisRange > IgnoreRange)
		return true;

	if (m_buttons[value] == POV00 && state.rgdwPOV[0] == 0
		|| m_buttons[value] == POV01 && state.rgdwPOV[0] == 9000
		|| m_buttons[value] == POV02 && state.rgdwPOV[0] == 18000
		|| m_buttons[value] == POV03 && state.rgdwPOV[0] == 27000
		|| m_buttons[value] == POV10 && state.rgdwPOV[1] == 0
		|| m_buttons[value] == POV11 && state.rgdwPOV[1] == 9000
		|| m_buttons[value] == POV12 && state.rgdwPOV[1] == 18000
		|| m_buttons[value] == POV13 && state.rgdwPOV[1] == 27000
		|| m_buttons[value] == POV20 && state.rgdwPOV[2] == 0
		|| m_buttons[value] == POV21 && state.rgdwPOV[2] == 9000
		|| m_buttons[value] == POV22 && state.rgdwPOV[2] == 18000
		|| m_buttons[value] == POV23 && state.rgdwPOV[2] == 27000
		|| m_buttons[value] == POV30 && state.rgdwPOV[3] == 0
		|| m_buttons[value] == POV31 && state.rgdwPOV[3] == 9000
		|| m_buttons[value] == POV32 && state.rgdwPOV[3] == 18000
		|| m_buttons[value] == POV33 && state.rgdwPOV[3] == 27000)
		return true;

	return false;
}

bool INScene::GetDeviceState(IDirectInputDevice8* pDevice,
							 unsigned int size, void* ptr)
{
	if (!pDevice)
		return false;
	for (int i = 0; i < GET_STATE_RETRIES; ++i)
	{
		HRESULT result = pDevice->GetDeviceState(size, ptr);
		if (SUCCEEDED(result))
			return true;
		if (result != DIERR_INPUTLOST && result != DIERR_NOTACQUIRED)
			throw new exception("Cannot acquire device"); //error! throw exeption
		for (int j = 0; j < ACQUIRE_RETRIES; ++j)
		{
			result = pDevice->Acquire();
			if (SUCCEEDED(result))
				break;
			if (result != DIERR_INPUTLOST && result != E_ACCESSDENIED)
				throw new exception("Cannot acquire device"); //error! throw exeption
		}
	}
	return false;
}

void INScene::RenderText()
{
	wstringstream str;
	str << L"FPS: " << m_counter.getCount();
	m_font->DrawString(m_context.get(), str.str().c_str(), 20.0f, 10.0f, 10.0f, 0xff0099ff, FW1_RESTORESTATE | FW1_NOGEOMETRYSHADER);
	if (DistanceToDoor() < 1.0f && FacingDoor())
	{
		wstring prompt(L"(E) Otwórz/Zamknij");
		FW1_RECTF layout;
		auto rect = m_font->MeasureString(prompt.c_str(), L"Calibri", 20.0f, &layout, FW1_NOWORDWRAP);
		float width = rect.Right - rect.Left;
		float height = rect.Bottom - rect.Top;
		auto clSize = m_window.getClientSize();
		m_font->DrawString(m_context.get(), prompt.c_str(), 20.0f, (clSize.cx - width) / 2, (clSize.cy - height) / 2, 0xff00ff99, FW1_RESTORESTATE | FW1_NOGEOMETRYSHADER);
	}
}

bool INScene::Initialize()
{
	if (!DxApplication::Initialize())
		return false;
	XFileLoader xloader(m_device);
	xloader.Load("house.x");
	m_sceneGraph.reset(new SceneGraph(move(xloader.m_nodes), move(xloader.m_meshes), move(xloader.m_materials)));

	m_doorNode = m_sceneGraph->nodeByName("Door");
	m_doorTransform = m_sceneGraph->getNodeTransform(m_doorNode);
	m_doorAngle = 0;
	m_doorAngVel = -XM_PIDIV2;

	m_cbProj.reset(new ConstantBuffer<XMFLOAT4X4>(m_device));
	m_cbView.reset(new ConstantBuffer<XMFLOAT4X4>(m_device));
	m_cbModel.reset(new ConstantBuffer<XMFLOAT4X4, 2>(m_device));
	m_cbMaterial.reset(new ConstantBuffer<Material::MaterialData>(m_device));

	EffectLoader eloader(m_device);
	eloader.Load(L"textured.hlsl");
	m_texturedEffect.reset(new TexturedEffect(move(eloader.m_vs), move(eloader.m_ps), m_cbProj, m_cbView, m_cbModel, m_cbMaterial));
	D3D11_INPUT_ELEMENT_DESC elem[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_layout = m_device.CreateInputLayout(elem, 3, eloader.m_vsCode);

	SIZE s = m_window.getClientSize();
	float ar = static_cast<float>(s.cx) / s.cy;
	XMStoreFloat4x4(&m_proj, XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), ar, 0.01f, 100.0f));
	m_cbProj->Update(m_context, m_proj);

	vector<OrientedBoundingRectangle> obstacles;
	obstacles.push_back(OrientedBoundingRectangle(XMFLOAT2(-3.0f, 3.8f), 6.0f, 0.2f, 0.0f));
	obstacles.push_back(OrientedBoundingRectangle(XMFLOAT2(-3.0f, -4.0f), 6.0f, 0.2f, 0.0f));
	obstacles.push_back(OrientedBoundingRectangle(XMFLOAT2(2.8f, -3.8f), 0.2f, 7.6f, 0.0f));
	obstacles.push_back(OrientedBoundingRectangle(XMFLOAT2(-3.0f, -3.8f), 0.2f, 4.85f, 0.0f));
	obstacles.push_back(OrientedBoundingRectangle(XMFLOAT2(-3.0f, 1.95f), 0.2f, 1.85f, 0.0f));
	obstacles.push_back(OrientedBoundingRectangle(XMFLOAT2(-3.05f, 1.0f), 0.1f, 1.0f, 0.0f));
	m_collisions.SetObstacles(move(obstacles));

	RasterizerDescription rsDesc;
	m_rsState = m_device.CreateRasterizerState(rsDesc);
	SamplerDescription sDesc;
	m_sampler = m_device.CreateSamplerState(sDesc);
	ID3D11SamplerState* sstates[1] = { m_sampler.get() };
	m_context->PSSetSamplers(0, 1, sstates);

	IFW1Factory *pf;
	HRESULT result = FW1CreateFactory(FW1_VERSION, &pf);
	m_fontFactory.reset(pf);
	if (FAILED(result))
		THROW_DX11(result);
	IFW1FontWrapper *pfw;
	result = m_fontFactory->CreateFontWrapper(m_device.getDevicePtr(), L"Calibri", &pfw);
	m_font.reset(pfw);
	if (FAILED(result))
		THROW_DX11(result);

	InitializeInput();
	return true;
}

void INScene::Render()
{
	if (!m_context)
		return;
	DxApplication::Render();
	m_context->RSSetState(m_rsState.get());
	XMFLOAT4X4 mtx[2];
	XMStoreFloat4x4(&mtx[0], m_camera.GetViewMatrix());
	m_cbView->Update(m_context, mtx[0]);
	m_texturedEffect->Begin(m_context);
	for (unsigned int i = 0; i < m_sceneGraph->meshCount(); ++i)
	{
		Mesh& m = m_sceneGraph->getMesh(i);
		Material& material = m_sceneGraph->getMaterial(m.getMaterialIdx());
		if (!material.getDiffuseTexture())
			continue;
		ID3D11ShaderResourceView* srv[2] = { material.getDiffuseTexture().get(), material.getSpecularTexture().get() };
		m_context->PSSetShaderResources(0, 2, srv);
		mtx[0] = m.getTransform();
		XMMATRIX modelit = XMLoadFloat4x4(&mtx[0]);
		XMVECTOR det;
		modelit = XMMatrixTranspose(XMMatrixInverse(&det, modelit));
		XMStoreFloat4x4(&mtx[1], modelit);
		m_cbMaterial->Update(m_context, material.getMaterialData());
		m_cbModel->Update(m_context, mtx);
		m_context->IASetInputLayout(m_layout.get());
		m.Render(m_context);
	}
	RenderText();
	RenderControlerMenu();
	RenderButtons();
}

void INScene::OpenDoor()
{
	if (m_doorAngVel < 0)
		m_doorAngVel = -m_doorAngVel;
}

void INScene::CloseDoor()
{
	if (m_doorAngVel > 0)
		m_doorAngVel = -m_doorAngVel;
}

void INScene::ToggleDoor()
{
	m_doorAngVel = -m_doorAngVel;
}

void INScene::UpdateDoor(float dt)
{
	if ((m_doorAngVel > 0 && m_doorAngle < XM_PIDIV2) || (m_doorAngVel < 0 && m_doorAngle > 0))
	{
		m_doorAngle += dt*m_doorAngVel;
		if (m_doorAngle < 0)
			m_doorAngle = 0;
		else if (m_doorAngle > XM_PIDIV2)
			m_doorAngle = XM_PIDIV2;
		XMFLOAT4X4 doorTransform;
		XMMATRIX mtx = XMLoadFloat4x4(&m_doorTransform);
		XMVECTOR v = XMVectorSet(0.000004f, 0.0f, -1.08108f, 1.0f);
		v = XMVector3TransformCoord(v, mtx);
		XMStoreFloat4x4(&doorTransform, mtx*XMMatrixTranslationFromVector(-v)*XMMatrixRotationZ(m_doorAngle)*XMMatrixTranslationFromVector(v));
		m_sceneGraph->setNodeTransform(m_doorNode, doorTransform);
		XMFLOAT2 tr = m_collisions.MoveObstacle(5, OrientedBoundingRectangle(XMFLOAT2(-3.05f, 1.0f), 0.1f, 1.0f, m_doorAngle));
		m_camera.Move(XMFLOAT3(tr.x, 0, tr.y));
		if (m_doorAngle == 0 || m_doorAngle == XM_PIDIV2)
			m_isReturnDown = false;
	}
}

void INScene::MoveCharacter(float dx, float dz)
{
	XMVECTOR forward = m_camera.getForwardDir();
	XMVECTOR right = m_camera.getRightDir();
	XMFLOAT3 temp;
	XMStoreFloat3(&temp, forward*dz + right*dx);
	XMFLOAT2 tr = XMFLOAT2(temp.x, temp.z);
	m_collisions.MoveCharacter(tr);
	m_camera.Move(XMFLOAT3(tr.x, 0, tr.y));
}

bool INScene::FacingDoor()
{
	auto rect = m_collisions.getObstacle(5);
	XMVECTOR points[4] = { XMLoadFloat2(&rect.getP1()), XMLoadFloat2(&rect.getP2()), XMLoadFloat2(&rect.getP3()), XMLoadFloat2(&rect.getP4()) };
	XMVECTOR forward = XMVectorSwizzle(m_camera.getForwardDir(), 0, 2, 1, 3);
	XMVECTOR camera = XMVectorSwizzle(XMLoadFloat4(&m_camera.getPosition()), 0, 2, 1, 3);
	unsigned int max_i = 0, max_j = 0;
	float max_a = 0.0f;
	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = i + 1; j < 4; ++j)
		{
			float a = XMVector2AngleBetweenVectors(points[i] - camera, points[j] - camera).m128_f32[0];
			if (a > max_a)
			{
				max_i = i;
				max_j = j;
				max_a = a;
			}
		}
	}
	return XMScalarNearEqual(XMVector2AngleBetweenVectors(forward, points[max_i] - camera).m128_f32[0] + XMVector2AngleBetweenVectors(forward, points[max_j] - camera).m128_f32[0], max_a, 0.001f);
}

float INScene::DistanceToDoor()
{
	return m_collisions.DistanceToObstacle(5);
}

void INScene::ChooseControler(BYTE keyboardState[256])
{
	if (m_controlerNumber > 0)
		if (keyboardState[m_buttons[Up]] && !(m_lastPressedButton == Up))
		{
			m_highlitedIndex = (m_highlitedIndex - 1 + m_controlerNumber) % m_controlerNumber;
			SetLastPressedButton(Up);
			return;
		}
		else if (keyboardState[m_buttons[Down]] && !(m_lastPressedButton == Down))
		{
			m_highlitedIndex = (m_highlitedIndex + 1) % m_controlerNumber;
			SetLastPressedButton(Down);
			return;
		}
		else if (keyboardState[m_buttons[Return]] && !(m_lastPressedButton == Return))
		{
			m_chosenControler = m_highlitedIndex;
			m_showControlers = false;
			m_renderButtons = true;
			m_maxButtonIndex = 4 * (m_highlitedIndex + 1) + 1;
			m_highlitedIndex = 0;
			SetLastPressedButton(Return);
		}
		else if (keyboardState[m_buttons[Menu]] && !(m_lastPressedButton == Menu))
		{
			m_showControlers = false;
			SetLastPressedButton(Menu);
			m_highlitedIndex = 0;
		}
}

void INScene::ChooseControlerJoystick(DIJOYSTATE2 state)
{
	if (m_controlerNumber > 0)
		if (CheckJoystickState(state, Up) && !(m_lastPressedButton == Up))
		{
			m_highlitedIndex = (m_highlitedIndex - 1 + m_controlerNumber) % m_controlerNumber;
			SetLastPressedButton(Up);
			return;
		}
		else if (CheckJoystickState(state, Down) && !(m_lastPressedButton == Down))
		{
			m_highlitedIndex = (m_highlitedIndex + 1) % m_controlerNumber;
			SetLastPressedButton(Down);
			return;
		}
		else if (CheckJoystickState(state, Return) && !(m_lastPressedButton == Return))
		{
			m_chosenControler = m_highlitedIndex;
			m_showControlers = false;
			m_renderButtons = true;
			m_maxButtonIndex = 4 * (m_highlitedIndex + 1) + 1;
			m_highlitedIndex = 0;
			SetLastPressedButton(Return);
		}
		else if (CheckJoystickState(state, Menu) && !(m_lastPressedButton == Menu))
		{
			m_showControlers = false;
			m_highlitedIndex = 0;
			SetLastPressedButton(Menu);
		}
}

void INScene::SetLastPressedButton(int button)
{
	m_lastPressedButton = button;
	m_timeLastPressElapsed = 0;
}

void INScene::ChooseButton(BYTE keyboardState[256])
{
	for (int i = 0; i < 256; i++)
	{
		if (keyboardState[i])
		{
			if (CheckPreviousButtons(m_highlitedIndex, i))break;
			m_buttons[m_highlitedIndex] = i;
			m_highlitedIndex++;
			if (m_highlitedIndex > m_maxButtonIndex)
				m_renderButtons = false;
			return;
		}
	}
}

void INScene::ChooseButtonJoystick(DIJOYSTATE2 state)
{
	for (int i = 0; i < m_capabilities.dwButtons; i++)
	{
		if (state.rgbButtons[i])
		{
			SetButton(i);
			return;
		}
	}
	for (int i = 0; i < m_capabilities.dwPOVs; i++)
	{
		if (state.rgdwPOV[i] != 4294967295)
		{
			int value = -400 - (10 * i) - ((int)state.rgdwPOV[i] / 9000);
			SetButton(value);
			return;
		}
	}
	if (state.lX > 0 && state.lX - MaxAxisRange > IgnoreRange)
		SetButton(XAxisDown);
	else if (state.lX > 0 && state.lX - MaxAxisRange < -IgnoreRange)
		SetButton(XAxisUp);
	else if (state.lY > 0 && state.lY - MaxAxisRange > IgnoreRange)
		SetButton(YAxisDown);
	else if (state.lY > 0 && state.lY - MaxAxisRange < -IgnoreRange)
		SetButton(YAxisUp);
	else if (state.lZ > 0 && state.lZ - MaxAxisRange > IgnoreRange)
		SetButton(ZAxisDown);
	else if (state.lZ > 0 && state.lZ - MaxAxisRange < -IgnoreRange)
		SetButton(ZAxisUp);
	else if (state.lRx > 0 && state.lRx - MaxAxisRange > IgnoreRange)
		SetButton(XAxisRotateRight);
	else if (state.lRx > 0 && state.lRx - MaxAxisRange < -IgnoreRange)
		SetButton(XAxisRotateLeft);
	else if (state.lRy > 0 && state.lRy - MaxAxisRange > IgnoreRange)
		SetButton(YAxisRotateRight);
	else if (state.lRy > 0 && state.lRy - MaxAxisRange < -IgnoreRange)
		SetButton(YAxisRotateLeft);
	else if (state.lRz > 0 && state.lRz - MaxAxisRange > IgnoreRange)
		SetButton(ZAxisRotateRight);
	else if (state.lRz > 0 && state.lRz - MaxAxisRange < -IgnoreRange)
		SetButton(ZAxisRotateLeft);
	else if (state.rglSlider > 0 && state.rglSlider[0] > 0 && state.rglSlider[0] - MaxAxisRange > IgnoreRange)
		SetButton(SliderDown);
	else if (state.rglSlider > 0 && state.rglSlider[0] > 0 && state.rglSlider[0] - MaxAxisRange < -IgnoreRange)
		SetButton(SliderUp);
	else if (state.rglSlider > 0 && state.rglSlider[1] > 0 && state.rglSlider[1] - MaxAxisRange > IgnoreRange)
		SetButton(SliderDown);
	else if (state.rglSlider > 0 && state.rglSlider[1] > 0 && state.rglSlider[1] - MaxAxisRange < -IgnoreRange)
		SetButton(SliderUp);
}

void INScene::SetButton(int value)
{
	if (CheckPreviousButtons(m_highlitedIndex, value) || m_lastPressedButton == m_highlitedIndex)return;
	m_buttons[m_highlitedIndex] = value;
	SetLastPressedButton(m_highlitedIndex);
	m_highlitedIndex++;
	if (m_highlitedIndex > m_maxButtonIndex)
	{
		m_renderButtons = false;
		m_highlitedIndex = 0;
	}
}

bool INScene::CheckPreviousButtons(int currentIndex, int key)
{
	for (int i = 0; i < currentIndex; i++)
	{
		if (m_buttons[i] == key)
			return true;
	}
	return false;
}

void INScene::RenderControlerMenu()
{
	if (!m_showControlers) return;

	RECT tarWnd;
	GetClientRect(m_window.getHandle(), &tarWnd);
	int left = (tarWnd.right - tarWnd.left) / 2 - 100;
	int top = (tarWnd.bottom - tarWnd.top) / 2 - 150;

	wstringstream str;
	str << L"Choose the controler.\nUse UP/DOWN and RETURN buttons:\n";
	m_font->DrawString(m_context.get(), str.str().c_str(), 20.0f, left, top, 0xf500992f, FW1_RESTORESTATE | FW1_NOGEOMETRYSHADER);
	top += 60;

	m_controlerNumber = 0;
	if (RenderControlerMenuKeyboard(left, top))
		top += 30;

	RenderControlerMenuJoystick(left, top);
}

bool INScene::RenderControlerMenuKeyboard(int left, int top)
{
	byte keyboardState[256];
	if (GetDeviceState(keyboard, sizeof(BYTE)* 256, &keyboardState))
	{
		UINT32 color = m_controlerNumber == m_highlitedIndex ? 0xffff992f : 0xf500992f;
		wstringstream keyboardStr;
		keyboardStr << ++m_controlerNumber;
		keyboardStr << ". KEYBOARD + MOUSE\n";
		m_font->DrawString(m_context.get(), keyboardStr.str().c_str(), 20.0f, left, top, color, FW1_RESTORESTATE | FW1_NOGEOMETRYSHADER);
		return true;
	}
	return false;
}

bool INScene::RenderControlerMenuJoystick(int left, int top)
{
	DIJOYSTATE2 state;
	if (GetDeviceState(joystick, sizeof(DIJOYSTATE2), &state))
	{
		UINT32 color = m_controlerNumber == m_highlitedIndex ? 0xffff992f : 0xf500992f;
		wstringstream joystickStr;
		joystickStr << ++m_controlerNumber;
		joystickStr << ". JOYSTICK\n";
		m_font->DrawString(m_context.get(), joystickStr.str().c_str(), 20.0f, left, top, color, FW1_RESTORESTATE | FW1_NOGEOMETRYSHADER);
		return true;
	}
	return false;
}

void INScene::RenderButtons()
{
	if (!m_renderButtons || m_highlitedIndex > m_maxButtonIndex) return;

	RECT tarWnd;
	GetClientRect(m_window.getHandle(), &tarWnd);
	int left = (tarWnd.right - tarWnd.left) / 2 - 100;
	int top = (tarWnd.bottom - tarWnd.top) / 2 - 150;

	wstringstream str;
	str << L"Choose the button corresponding to ";
	str << m_buttonsStrings[m_highlitedIndex];

	m_font->DrawString(m_context.get(), str.str().c_str(), 20.0f, left, top, 0xf500992f, FW1_RESTORESTATE | FW1_NOGEOMETRYSHADER);
}