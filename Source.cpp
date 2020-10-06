#include "Include/d3dx9math.h"
#include "Include/DirectOverlay.h"

#define M_PI 3.14159265358979323846264338327950288419716939937510

#define OFFSET_UWORLD 0x748CC70 
#define location_Offs 0x7467240

bool Menu = true;
bool Aimbot = true;
bool BoxESP = true;
bool EnemyESP = true;
bool LineESP = true;
bool DistanceESP = true;
bool BoneESP = true;
bool FOV = true;
bool visuals = false;
bool aimbots = false;
bool miscs = false;
bool AimbotFov = true;

DWORD processID;
HWND hwnd = NULL;

int width;
int height;
int localplayerID;

HANDLE Driver_File;
uint64_t base_address;

DWORD_PTR Uworld;
DWORD_PTR LocalPawn;
DWORD_PTR Localplayer;
DWORD_PTR Rootcomp;
DWORD_PTR PlayerController;
DWORD_PTR Ulevel;
//Bypass anticheats


FTransform GetBoneIndex(DWORD_PTR mesh, int index)
{
	DWORD_PTR bonearray = read<DWORD_PTR>(Driver_File, processID, mesh + 0x410);

	return read<FTransform>(Driver_File, processID, bonearray + (index * 0x30));
}

Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id)
{
	FTransform bone = GetBoneIndex(mesh, id);
	FTransform ComponentToWorld = read<FTransform>(Driver_File, processID, mesh + 0x1C0);

	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

Vector3 Camera(unsigned __int64 RootComponent)
{
	unsigned __int64 PtrPitch;
	Vector3 Camera;

	Camera.x = read<float>(Driver_File, processID, RootComponent + 0x12C);
	auto pitch = read<uintptr_t>(Driver_File, processID, Localplayer + 0xb0);

	Camera.y = read<float>(Driver_File, processID, pitch + 0x678);

	float test = asin(Camera.y);
	float degrees = test * (180.0 / M_PI);

	Camera.y = degrees;

	if (Camera.x < 0)
		Camera.x = 360 + Camera.x;

	return Camera;
}

D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

Vector3 ProjectWorldToScreen(Vector3 WorldLocation, Vector3 camrot)
{
	Vector3 Screenlocation = Vector3(0, 0, 0);
	Vector3 Rotation = camrot; // FRotator

	D3DMATRIX tempMatrix = Matrix(Rotation);

	Vector3 vAxisX, vAxisY, vAxisZ;

	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 camloc = read<Vector3>(Driver_File, processID, base_address + location_Offs);

	Vector3 vDelta = WorldLocation - camloc;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	uint64_t zoomBase = read<uint64_t>(Driver_File, processID, Localplayer + 0xb0);
	float zoom = read<float>(Driver_File, processID, zoomBase + 0x500);

	float FovAngle = 80.0f / (zoom / 1.19f);
	float ScreenCenterX = width / 2.0f;
	float ScreenCenterY = height / 2.0f;

	Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;

	return Screenlocation;
}

void menu()
{
	if (Menu)
	{
		// Draw the fancy stuff OwO
		DrawBox(5.f, 5.f, 455.f, 300.f, 0.f, 0.f, 0.f, 0.f, 0.7f, true);
		DrawBox(3, 3, 455, 302, 2.0f, 255, 255, 255, 100, false);
		DrawLine(3, 40, 455, 40, 2.0f, 255, 255, 255, 100);
		DrawString(_xor_("Scarlet FN Cheat").c_str(), 18, 145, 10, 129.f, 202.f, 0.f, 24.f);
		// Visuals start
		if (visuals)
		{
			DrawString(_xor_("ON").c_str(), 13, 10 + 115, 10 + 40, 0.f, 104.f, 56.f, 255.f);
			if (BoxESP)
				DrawString(_xor_("ON").c_str(), 13, 10 + 150, 10 + 60, 0.f, 255.f, 0.f, 255.f);
			else
				DrawString(_xor_("OFF").c_str(), 13, 10 + 150, 10 + 60, 255.f, 0.f, 0.f, 255.f);

			DrawString(_xor_("[F2] Box ESP").c_str(), 13, 10, 10 + 60, 255.f, 255.f, 255.f, 255.f);

			if (LineESP)
				DrawString(_xor_("ON").c_str(), 13, 10 + 150, 10 + 80, 0.f, 255.f, 0.f, 255.f);
			else
				DrawString(_xor_("OFF").c_str(), 13, 10 + 150, 10 + 80, 255.f, 0.f, 0.f, 255.f);

			DrawString(_xor_("[F3] SnapLine ESP").c_str(), 13, 10, 10 + 80, 255.f, 255.f, 255.f, 255.f);


			if (DistanceESP)
				DrawString(_xor_("ON").c_str(), 13, 10 + 150, 10 + 100, 0.f, 255.f, 0.f, 255.f);
			else
				DrawString(_xor_("OFF").c_str(), 13, 10 + 150, 10 + 100, 255.f, 0.f, 0.f, 255.f);

			DrawString(_xor_("[F4] Distance ESP").c_str(), 13, 10, 10 + 100, 255.f, 255.f, 255.f, 255.f);

			if (BoneESP)
				DrawString(_xor_("ON").c_str(), 13, 10 + 150, 10 + 120, 0.f, 255.f, 0.f, 255.f);
			else
				DrawString(_xor_("OFF").c_str(), 13, 10 + 150, 10 + 120, 255.f, 0.f, 0.f, 255.f);

			DrawString(_xor_("[F6] Bones ESP").c_str(), 13, 10, 10 + 120, 255.f, 255.f, 255.f, 255.f);
		}
		else
			DrawString(_xor_("OFF").c_str(), 13, 10 + 115, 10 + 40, 255.f, 0.f, 0.f, 255.f);

		DrawString(_xor_("Visual Options F8").c_str(), 13, 10, 10 + 40, 255.f, 255.f, 255.f, 255.f);
		// Visuals end

		// Aimbot start
		if (aimbots)
		{
			DrawString(_xor_("ON").c_str(), 13, 10 + 275, 10 + 40, 0.f, 104.f, 56.f, 255.f);

			if (Aimbot)
				DrawString(_xor_("ON").c_str(), 13, 10 + 150, 10 + 140, 0.f, 255.f, 0.f, 255.f);
			else
				DrawString(_xor_("OFF").c_str(), 13, 10 + 150, 10 + 140, 255.f, 0.f, 0.f, 255.f);

			DrawString(_xor_("[F5] Aimbot").c_str(), 13, 10, 10 + 140, 255.f, 255.f, 255.f, 255.f);
		}
		else
			DrawString(_xor_("OFF").c_str(), 13, 10 + 275, 10 + 40, 255.f, 0.f, 0.f, 255.f);

		DrawString(_xor_("Aimbot Options F9").c_str(), 13, 165, 10 + 40, 255.f, 255.f, 255.f, 255.f);
		// Aimbot end

		// Mics start
		if (miscs)
		{
			DrawString(_xor_("ON").c_str(), 13, 10 + 420, 10 + 40, 0.f, 104.f, 56.f, 255.f);

			if (AimbotFov)
				DrawString(_xor_("ON").c_str(), 13, 10 + 150, 10 + 180, 0.f, 255.f, 0.f, 255.f);
			else
				DrawString(_xor_("OFF").c_str(), 13, 10 + 150, 10 + 180, 255.f, 0.f, 0.f, 255.f);

			DrawString(_xor_("[F1] Fov circle").c_str(), 13, 10, 10 + 180, 255.f, 255.f, 255.f, 255.f);
		}
		else
			DrawString(_xor_("OFF").c_str(), 13, 10 + 420, 10 + 40, 255.f, 0.f, 0.f, 255.f);

		DrawString(_xor_("Misc Options F10").c_str(), 13, 320, 10 + 40, 255.f, 255.f, 255.f, 255.f);
		// Misc end
	}
}

DWORD Menuthread(LPVOID in)
{
	while (1)
	{
		if (GetAsyncKeyState(VK_INSERT) & 1) {
			Menu = !Menu;
		}

		if (Menu)
		{
			if (GetAsyncKeyState(VK_F1) & 1) {
				AimbotFov = !AimbotFov;
			} 

			if (GetAsyncKeyState(VK_F2) & 1) {
				EnemyESP = !EnemyESP;
			}

			if (GetAsyncKeyState(VK_F2) & 1) {
				BoxESP = !BoxESP;
			}

			if (GetAsyncKeyState(VK_F3) & 1) {
				LineESP = !LineESP;
			}

			if (GetAsyncKeyState(VK_F4) & 1) {
				DistanceESP = !DistanceESP;
			}

			if (GetAsyncKeyState(VK_F5) & 1) {
				Aimbot = !Aimbot;
			}
			if (GetAsyncKeyState(VK_F6) & 1) {
				BoneESP = !BoneESP;
			}
			if (GetAsyncKeyState(VK_F8) & 1) {
				visuals = !visuals;
			}

			if (GetAsyncKeyState(VK_F9) & 1) {
				aimbots = !aimbots;
			}
			if (GetAsyncKeyState(VK_F10) & 1) {
				miscs = !miscs;
			}
		}
	}
}

void drawLoop(int width, int height) {
	menu();

	Uworld = read<DWORD_PTR>(Driver_File, processID, base_address + OFFSET_UWORLD);
	//printf(_xor_("Uworld: %p.\n").c_str(), Uworld);

	DWORD_PTR Gameinstance = read<DWORD_PTR>(Driver_File, processID, Uworld + 0x170);

	if (Gameinstance == (DWORD_PTR)nullptr)
		return;

	//printf(_xor_("Gameinstance: %p.\n").c_str(), Gameinstance);

	DWORD_PTR LocalPlayers = read<DWORD_PTR>(Driver_File, processID, Gameinstance + 0x38);

	if (LocalPlayers == (DWORD_PTR)nullptr)
		return;

	//	printf(_xor_("LocalPlayers: %p.\n").c_str(), LocalPlayers);

	Localplayer = read<DWORD_PTR>(Driver_File, processID, LocalPlayers);

	if (Localplayer == (DWORD_PTR)nullptr)
		return;

	//printf(_xor_("LocalPlayer: %p.\n").c_str(), Localplayer);

	PlayerController = read<DWORD_PTR>(Driver_File, processID, Localplayer + 0x30);

	if (PlayerController == (DWORD_PTR)nullptr)
		return;

	//printf(_xor_("playercontroller: %p.\n").c_str(), PlayerController);

	LocalPawn = read<uint64_t>(Driver_File, processID, PlayerController + 0x298);

	if (LocalPawn == (DWORD_PTR)nullptr)
		return;

	//printf(_xor_("Pawn: %p.\n").c_str(), LocalPawn);

	Rootcomp = read<uint64_t>(Driver_File, processID, LocalPawn + 0x130);

	if (Rootcomp == (DWORD_PTR)nullptr)
		return;

	//printf(_xor_("Rootcomp: %p.\n").c_str(), Rootcomp);

	if (LocalPawn != 0) {
		localplayerID = read<int>(Driver_File, processID, LocalPawn + 0x18);
		//std::cout << _xor_("localplayerID = ").c_str() << localplayerID << std::endl;
	}

	Ulevel = read<DWORD_PTR>(Driver_File, processID, Uworld + 0x30);
	//printf(_xor_("Ulevel: %p.\n").c_str(), Ulevel);

	if (Ulevel == (DWORD_PTR)nullptr)
		return;

	DWORD ActorCount = read<DWORD>(Driver_File, processID, Ulevel + 0xA0);
	//printf(_xor_("ActorCount: %p.\n").c_str(), ActorCount);

	DWORD_PTR AActors = read<DWORD_PTR>(Driver_File, processID, Ulevel + 0x98);
	//printf(_xor_("AActors: %p.\n").c_str(), AActors);

	if (AActors == (DWORD_PTR)nullptr)
		return;

	for (int i = 0; i < ActorCount; i++)
	{


		uint64_t CurrentActor = read<uint64_t>(Driver_File, processID, AActors + i * 0x8);

		if (CurrentActor == (uint64_t)nullptr || CurrentActor == -1 || CurrentActor == NULL)
			continue;

		uint64_t LocalPlayers_TeamID = read<uint64_t>(Driver_File, processID, Gameinstance + 0xE20);

		int curactorid = read<int>(Driver_File, processID, CurrentActor + 0x18);

		if (curactorid == localplayerID || curactorid == 16789273)
		{
			uint64_t CurrentActorRootComponent = read<uint64_t>(Driver_File, processID, CurrentActor + 0x130);

			if (CurrentActorRootComponent == (uint64_t)nullptr || CurrentActorRootComponent == -1 || CurrentActorRootComponent == NULL)
				continue;

			uint64_t currentactormesh = read<uint64_t>(Driver_File, processID, CurrentActor + 0x278);

			if (currentactormesh == (uint64_t)nullptr || currentactormesh == -1 || currentactormesh == NULL)
				continue;

			Vector3 Headpos = GetBoneWithRotation(currentactormesh, 66);
			Vector3 Localcam = Camera(Rootcomp);
			Vector3 localactorpos = read<Vector3>(Driver_File, processID, Rootcomp + 0x11C);
			float distance = localactorpos.Distance(Headpos) / 100.f;

			if (distance < 1.5f)
				continue;

			//W2S Bones
			Vector3 HeadposW2s = ProjectWorldToScreen(Headpos, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 bone0 = GetBoneWithRotation(currentactormesh, 0);
			Vector3 bottom = ProjectWorldToScreen(bone0, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 Headbox = ProjectWorldToScreen(Vector3(Headpos.x, Headpos.y, Headpos.z + 15), Vector3(Localcam.y, Localcam.x, Localcam.z));

			Vector3 neckpos = GetBoneWithRotation(currentactormesh, 65);
			Vector3 pelvispos = GetBoneWithRotation(currentactormesh, 3);
			Vector3 BONE_L_ELBOW = GetBoneWithRotation(currentactormesh, 10);
			Vector3 BONE_R_ELBOW = GetBoneWithRotation(currentactormesh, 38);
			Vector3 BONE_L_SHOULDER = GetBoneWithRotation(currentactormesh, 9);
			Vector3 BONE_R_SHOULDER = GetBoneWithRotation(currentactormesh, 62);
			Vector3 BONE_R_WRIST = GetBoneWithRotation(currentactormesh, 60);
			Vector3 BONE_L_WRIST = GetBoneWithRotation(currentactormesh, 20);
			Vector3 BONE_L_HAND = GetBoneWithRotation(currentactormesh, 32);
			Vector3 BONE_R_HAND = GetBoneWithRotation(currentactormesh, 59);
			Vector3 BONE_L_LEG_ROOT = GetBoneWithRotation(currentactormesh, 67);
			Vector3 BONE_R_LEG_ROOT = GetBoneWithRotation(currentactormesh, 74);
			Vector3 BONE_L_KNEE = GetBoneWithRotation(currentactormesh, 68);
			Vector3 BONE_R_KNEE = GetBoneWithRotation(currentactormesh, 75);
			Vector3 BONE_L_FOOT_ROOT = GetBoneWithRotation(currentactormesh, 71);
			Vector3 BONE_R_FOOT_ROOT = GetBoneWithRotation(currentactormesh, 78);
			Vector3 BONE_L_FOOT_LOW = GetBoneWithRotation(currentactormesh, 72);
			Vector3 BONE_R_FOOT_LOW = GetBoneWithRotation(currentactormesh, 79);



			Vector3 neckposW2s = ProjectWorldToScreen(neckpos, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 pelvisW2s = ProjectWorldToScreen(pelvispos, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_L_ELBOW_1W2s = ProjectWorldToScreen(BONE_L_ELBOW, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_R_ELBOW_1W2s = ProjectWorldToScreen(BONE_R_ELBOW, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_L_SHOULDER1W2s = ProjectWorldToScreen(BONE_L_SHOULDER, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_R_SHOULDER1W2s = ProjectWorldToScreen(BONE_R_SHOULDER, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_L_WRIST1W2s = ProjectWorldToScreen(BONE_L_WRIST, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_R_WRIST1W2s = ProjectWorldToScreen(BONE_R_WRIST, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_L_HAND1W2s = ProjectWorldToScreen(BONE_L_HAND, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_R_HAND1W2s = ProjectWorldToScreen(BONE_R_HAND, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_L_LEG_ROOT1W2s = ProjectWorldToScreen(BONE_L_LEG_ROOT, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_R_LEG_ROOT1W2s = ProjectWorldToScreen(BONE_R_LEG_ROOT, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_L_KNEE1W2s = ProjectWorldToScreen(BONE_L_KNEE, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_R_KNEE1W2s = ProjectWorldToScreen(BONE_R_KNEE, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_L_FOOT_ROOT1W2s = ProjectWorldToScreen(BONE_L_FOOT_ROOT, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_R_FOOT_ROOT1W2s = ProjectWorldToScreen(BONE_R_FOOT_ROOT, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_L_FOOT_LOW1W2s = ProjectWorldToScreen(BONE_L_FOOT_LOW, Vector3(Localcam.y, Localcam.x, Localcam.z));
			Vector3 BONE_R_FOOT_LOW1W2s = ProjectWorldToScreen(BONE_R_FOOT_LOW, Vector3(Localcam.y, Localcam.x, Localcam.z));

			Vector3 previous(0, 0, 0);
			Vector3 current, p1, n1;
			float Height1 = abs(Headbox.y - bottom.y);
			float Width1 = Height1 * 0.65;
			float ScreenCenterX = width / 2.0f;
			float ScreenCenterY = height / 2.0f;
			int MaxFov = 0;


			if (BoxESP)
				DrawBox(Headbox.x - (Width1 / 2), Headbox.y, Width1, Height1, 2.f, 0, 221, 255, 1, false);
			DrawBox(Headbox.x - (Width1 / 2), Headbox.y, Width1, Height1, 1.f, 0, 0, 0, 0.5, true);

			if (EnemyESP)
				DrawString(_xor_("Enemy").c_str(), 13, HeadposW2s.x, HeadposW2s.y - 25, 0, 1, 1);

			if (BoneESP)
			{
				int skeleton_color_r = 0;
				int skeleton_color_g = 255;
				int skeleton_color_b = 0;
				Height1 = (Headbox.y - HeadposW2s.y - 2);
				Width1 = (Height1 / 2);
				DrawLine(pelvisW2s.x, pelvisW2s.y, neckposW2s.x, neckposW2s.y, 2, skeleton_color_r, 255, 0, 1);
				DrawLine(neckposW2s.x, neckposW2s.y, BONE_L_SHOULDER1W2s.x, BONE_L_SHOULDER1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(neckposW2s.x, neckposW2s.y, BONE_R_SHOULDER1W2s.x, BONE_R_SHOULDER1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_R_SHOULDER1W2s.x, BONE_R_SHOULDER1W2s.y, BONE_R_ELBOW_1W2s.x, BONE_R_ELBOW_1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_L_SHOULDER1W2s.x, BONE_L_SHOULDER1W2s.y, BONE_L_ELBOW_1W2s.x, BONE_L_ELBOW_1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_R_ELBOW_1W2s.x, BONE_R_ELBOW_1W2s.y, BONE_R_WRIST1W2s.x, BONE_R_WRIST1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_L_ELBOW_1W2s.x, BONE_L_ELBOW_1W2s.y, BONE_L_WRIST1W2s.x, BONE_L_WRIST1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_L_WRIST1W2s.x, BONE_L_WRIST1W2s.y, BONE_L_HAND1W2s.x, BONE_L_HAND1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_R_WRIST1W2s.x, BONE_R_WRIST1W2s.y, BONE_R_HAND1W2s.x, BONE_R_HAND1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(pelvisW2s.x, pelvisW2s.y, BONE_L_LEG_ROOT1W2s.x, BONE_L_LEG_ROOT1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(pelvisW2s.x, pelvisW2s.y, BONE_R_LEG_ROOT1W2s.x, BONE_R_LEG_ROOT1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_L_KNEE1W2s.x, BONE_L_KNEE1W2s.y, BONE_L_LEG_ROOT1W2s.x, BONE_L_LEG_ROOT1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_R_KNEE1W2s.x, BONE_R_KNEE1W2s.y, BONE_R_LEG_ROOT1W2s.x, BONE_R_LEG_ROOT1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_L_KNEE1W2s.x, BONE_L_KNEE1W2s.y, BONE_L_FOOT_ROOT1W2s.x, BONE_L_FOOT_ROOT1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_R_KNEE1W2s.x, BONE_R_KNEE1W2s.y, BONE_R_FOOT_ROOT1W2s.x, BONE_R_FOOT_ROOT1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_L_FOOT_LOW1W2s.x, BONE_L_FOOT_LOW1W2s.y, BONE_L_FOOT_ROOT1W2s.x, BONE_L_FOOT_ROOT1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);
				DrawLine(BONE_R_FOOT_LOW1W2s.x, BONE_R_FOOT_LOW1W2s.y, BONE_R_FOOT_ROOT1W2s.x, BONE_R_FOOT_ROOT1W2s.y, 2, skeleton_color_r, skeleton_color_g, skeleton_color_b, 1);

				previous = current;
			}
			if (DistanceESP)
			{
				CHAR dist[50];
				sprintf_s(dist, _xor_("[%.f]").c_str(), distance);

				DrawString(dist, 13, HeadposW2s.x + 40, HeadposW2s.y - 25, 0, 1, 1);
			}
			if (AimbotFov)
			{
				MaxFov = 120;
			}
			else
				MaxFov = 0;

			if (LineESP)
				DrawLine(width / 2, height, bottom.x, bottom.y, 2.f, 0, 221, 255, 1);

			if (Aimbot)
			{
				float headX = HeadposW2s.x - width / 2;
				float headY = HeadposW2s.y - height / 2;

				float x = (headX - ScreenCenterX);
				float y = (headY - ScreenCenterY);

				float EnemyDistFromXhair = (float)sqrt((x * x) + (y * y));

				float maxDistance = 10;// max distance


				DrawCircle(ScreenCenterX, ScreenCenterY, MaxFov, 1.f, 0, 221, 255, 1, false);
				if (AimbotFov) 
				{
					if (headX >= -MaxFov && headX <= MaxFov && headY >= -MaxFov && headY <= MaxFov && distance < 500)
					{
						if (GetAsyncKeyState(VK_RBUTTON))
						{
							mouse_event(MOUSEEVENTF_MOVE, headX, headY, NULL, NULL);
						}
					}
				}
				else
				if (headX >= headX <= headY >= headY <= distance < 500)
				{
					if (GetAsyncKeyState(VK_RBUTTON))
					{
						mouse_event(MOUSEEVENTF_MOVE, headX, headY, NULL, NULL);
					}
				}

			}
		}
	}
	Sleep(2);
}

void main()
{
	while (hwnd == NULL)
	{
		hwnd = FindWindowA(0, _xor_("Fortnite  ").c_str());

		printf(_xor_("Scarlet FN (BETA)\n").c_str());
		 
		printf(_xor_("Discord = https://discord.gg/JD3JvwS\n").c_str());
		
		printf(_xor_("Looking for FortniteClient-Win64-Shipping.exe\n").c_str());
		Sleep(3000);
	}

	GetWindowThreadProcessId(hwnd, &processID);

	RECT rect;
	if (GetWindowRect(hwnd, &rect))
	{
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}

	Driver_File = CreateFileW(_xor_(L"\\\\.\\OVERHAX").c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (Driver_File == INVALID_HANDLE_VALUE)
	{
		printf(_xor_("Load Driver first, exiting...\n").c_str());
		Sleep(2000);
		exit(0);
	}

	info_t Input_Output_Data;
	Input_Output_Data.pid = processID;
	unsigned long int Readed_Bytes_Amount;

	DeviceIoControl(Driver_File, ctl_base, &Input_Output_Data, sizeof Input_Output_Data, &Input_Output_Data, sizeof Input_Output_Data, &Readed_Bytes_Amount, nullptr);
	base_address = (unsigned long long int)Input_Output_Data.data;

	std::printf(_xor_("Process base address: %p.\n").c_str(), (void*)base_address);

	CreateThread(NULL, NULL, Menuthread, NULL, NULL, NULL);

	DirectOverlaySetOption(D2DOV_DRAW_FPS | D2DOV_FONT_COURIER);
	DirectOverlaySetup(drawLoop, FindWindow(NULL, _xor_("Fortnite  ").c_str()));
	getchar();
}
