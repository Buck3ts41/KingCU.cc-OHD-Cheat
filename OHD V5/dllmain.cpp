// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "libs/SDK.h"
#include "libs/SDK/BasicTypes_Package.cpp"
#include "libs/SDK/CoreUObject_Package.cpp"
#include "libs/SDK/Engine_Package.cpp"
#include "libs/SDK/DonkehFramework_Package.cpp"
#include "libs/ZeroGUI/ZeroGUI.h";
#include "libs/ZeroGUI/ZeroInput.h"
#include <windows.h>
#include <ShellAPI.h>
#include <psapi.h>

#define KeyPressed( k ) ( GetAsyncKeyState(k) & 0x8000 )
#define Nullcheck(x) if (x == NULL || x == nullptr || !x) {return;}
#define NullcheckC(x) if (x == NULL || x == nullptr || !x) {continue;}

using namespace CG;
using namespace ZeroGUI;

inline FLinearColor Text_Shadow{ 0.0f, 0.0f, 0.0f, 0.0f };

inline int MenuKey = 45;

inline bool EnableAimbot = false;
inline bool AimbotVisibleOnly = false;
inline bool AimbotTargetTeam = false;
inline int AimbotKey = 1;

inline bool EnableESP = false;
inline bool Nametags = false;
inline bool Boxes = false;
inline bool cornerbox = false;
inline bool daytime = false;
inline bool nightmode = false;
inline bool nightmode2 = false;
inline bool fovc = false;
inline bool cross = false;
inline bool Skeletons = false;
inline bool Snaplines = false;
inline float SnaplinesY = 0.f;
inline float SnaplinesX = 960.f;
inline bool IgnoreTeamESP = false;
inline bool VisibleColors = false;
inline bool VisibleOnly = false;
inline FLinearColor ESPColor = FLinearColor{ 1.f, 0.f, 0.f, 1.f };

inline FLinearColor ESPTeamColor = FLinearColor{ 0.f, 1.f, 0.f, 1.f };
inline FLinearColor Circlecolor = FLinearColor{ 1.f, 0.f, 0.f, 1.f };
inline FLinearColor crosscolor = FLinearColor{ 1.f, 0.f, 0.f, 1.f };
inline FLinearColor ESPEnemyColor = FLinearColor{ 1.f, 0.f, 0.f, 1.f };
inline FLinearColor ESPVisibleColor = rgb2rgbfl(252, 232, 3);

inline bool InfiniteAmmo = false;
inline bool FullAuto = false;
inline bool NoRecoil = false;
inline bool FireRate = false;
inline float ShotsPerBurst = 3.0f;
inline int NukerKey = 86;
inline bool AutoNuke = false;
inline bool IgnoreTeamNuker = false;

inline bool Fly = false;
inline bool ResetFly = false;
inline bool Noclip = false;
inline bool Desync = false;
inline float Speed = 1.f;
inline float FOV = 90.f;

void GetDesktopResolution(float& horizontal, float& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

typedef FMatrix* (__thiscall* _GetBoneMatrix)(USkeletalMeshComponent* mesh, FMatrix* result, int index);
FVector get_bone_location_by_index(USkeletalMeshComponent* mesh, int index)
{
    if (!mesh)
        return { 0.f, 0.f, 0.f };

    FMatrix matrix{};

    _GetBoneMatrix fGetBoneMatrix = reinterpret_cast<_GetBoneMatrix>((uint64_t)GetModuleHandleA(NULL) + 0x218A460);
    fGetBoneMatrix(mesh, &matrix, index);

    return matrix.WPlane;
}

inline float horizontal = 0;
inline float vertical = 0;

void NullcheckWeapon(AHDPlayerCharacter * character, AHDBaseWeapon * weapon) {
    Nullcheck(character);
    Nullcheck(weapon);
    // useless but makes me thing i save time.
}

void Nuke() {
    UWorld** p_uworld = reinterpret_cast<UWorld**>(UWorld::GWorld);
    Nullcheck(p_uworld);
    Nullcheck(*p_uworld);
    UGameInstance* OwningGameInstance = (*p_uworld)->OwningGameInstance;
    Nullcheck(OwningGameInstance);
    auto GameState = (*p_uworld)->GameState;
    Nullcheck(GameState);
    UWorld* gworld = UWorld::GWorld[0];
    Nullcheck(gworld);
    ULocalPlayer* localplayer = gworld->OwningGameInstance->LocalPlayers[0];
    Nullcheck(localplayer);

    auto pArray = GameState->PlayerArray;
    if (pArray.Count() > 1) {
        for (USHORT i = 0; i < pArray.Count(); i++) {
            auto& ent = pArray[i];
            if (ent != localplayer->PlayerController->PlayerState) {
                auto Character = static_cast<AHDPlayerCharacter*>(ent->PawnPrivate);
                NullcheckC(Character);
                if (ent->PawnPrivate == nullptr) continue;
                if (ent->PawnPrivate == nullptr) continue;
                if (Character->Health <= 0) continue;
                AHDPlayerCharacter* SelfPlayer = static_cast<AHDPlayerCharacter*>(localplayer->PlayerController->AcknowledgedPawn);
                NullcheckC(SelfPlayer);
                if (IgnoreTeamNuker && Character->TeamNum != SelfPlayer->TeamNum) {
                    auto Location = Character->ReplicatedMovement.Location;
                    auto BaseWeapon = reinterpret_cast<AHDBaseWeapon*>(SelfPlayer->EquippedItem);
                    NullcheckC(BaseWeapon);
                    BaseWeapon->ServerFireProjectile(Location, {});
                    
                }
                else {
                    auto Location = Character->ReplicatedMovement.Location;
                    auto BaseWeapon = reinterpret_cast<AHDBaseWeapon*>(SelfPlayer->EquippedItem);
                    NullcheckC(BaseWeapon);
                    BaseWeapon->ServerFireProjectile(Location, {});
                }
            }
        }
    }
}
// Find the process ID of the target process by name
DWORD FindProcessId(const char* processName)
{
    DWORD processIds[1024], cbNeeded, count;
    if (!EnumProcesses(processIds, sizeof(processIds), &cbNeeded))
        return 0;

    count = cbNeeded / sizeof(DWORD);
    for (DWORD i = 0; i < count; i++)
    {
        if (processIds[i] != 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
            if (hProcess != NULL)
            {
                char szProcessName[MAX_PATH] = { 0 };
                if (GetModuleBaseNameA(hProcess, NULL, szProcessName, sizeof(szProcessName)) != 0)
                {
                    if (strcmp(szProcessName, processName) == 0)
                    {
                        CloseHandle(hProcess);
                        return processIds[i];
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
    return 0;
}
DWORD targetProcessId = FindProcessId("HarshDoorstop-Win64-Shipping.exe");

void ModifyInstruction(LPVOID address, BYTE* code, DWORD size)
{
    DWORD oldProtect;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(address, code, size);
    VirtualProtect(address, size, oldProtect, &oldProtect);
}

// The function that modifies the instruction in the target process
void nightON()
{ 
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessId);
    HMODULE hModule = GetModuleHandle("HarshDoorstop-Win64-Shipping.exe");
    LPVOID moduleBase = (LPVOID)hModule;
    
    // Calculate the address of the instruction we want to modify
    LPVOID instructionAddress = (LPVOID)((BYTE*)moduleBase + 0x253B504);

    // Modify the instruction
    BYTE code[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    ModifyInstruction(instructionAddress, code, sizeof(code));

    // Close the handle to the target process
    CloseHandle(hProcess);
}
void nightOFF()
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessId);
    HMODULE hModule = GetModuleHandle("HarshDoorstop-Win64-Shipping.exe");
    LPVOID moduleBase = (LPVOID)hModule;

    // Calculate the address of the instruction we want to modify
    LPVOID instructionAddress = (LPVOID)((BYTE*)moduleBase + 0x253B504);

    // Modify the instruction
    BYTE code[] = { 0x0F, 0x28, 0x05, 0xF5, 0x56, 0xA3, 0x00 };
    ModifyInstruction(instructionAddress, code, sizeof(code));

    // Close the handle to the target process
    CloseHandle(hProcess);
}

void tymeday()
{
    
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessId);
    HMODULE hModule = GetModuleHandle("HarshDoorstop-Win64-Shipping.exe");
    LPVOID moduleBase = (LPVOID)hModule;
    LPVOID instructionAddress = (LPVOID)((BYTE*)moduleBase + 0x253B466);

    // Modify the instruction
    BYTE code[] = { 0x90, 0x90, 0x90 };
    ModifyInstruction(instructionAddress, code, sizeof(code));

    // Close the handle to the target process
    CloseHandle(hProcess);
}


FVector2D pos = FVector2D{ 100.0f, 100.0f };
FVector2D WindowSize = FVector2D{ 500.0f, 240.0f };
inline bool infAmmoCheck = false;

void Tick()
{
    ZeroGUI::Input::Handle();

    static bool menu_opened = false;
    if (GetAsyncKeyState(MenuKey) & 1) menu_opened = !menu_opened; //Our menu key
    
    if (ZeroGUI::Window((char*)"Buck3ts41 Internal v2.1", &pos, WindowSize, menu_opened))
    {
        static int tab = 0;
        if (ZeroGUI::ButtonTab((char*)"Aimbot", FVector2D{ 100, 15 }, tab == 0)) tab = 0;
        ZeroGUI::SameLine();
        if (ZeroGUI::ButtonTab((char*)"ESP", FVector2D{ 100, 15 }, tab == 1)) tab = 1;
        ZeroGUI::SameLine();
        if (ZeroGUI::ButtonTab((char*)"Weapon", FVector2D{ 100, 15 }, tab == 2)) tab = 2;
        ZeroGUI::SameLine();
        if (ZeroGUI::ButtonTab((char*)"Miscellaneous", FVector2D{ 100, 15 }, tab == 3)) tab = 3;
        ZeroGUI::SameLine();
        if (tab == 0) {
            WindowSize = FVector2D{ 500.0f, 550.0f };
            ZeroGUI::Text((char*)"Aim");
            ZeroGUI::Checkbox((char*)"Enable Aimbot [need update]", &EnableAimbot);
            
            ZeroGUI::Checkbox((char*)"Visible Only [need update]", &AimbotVisibleOnly);
            
            ZeroGUI::Checkbox((char*)"Target Team [need update]", &AimbotTargetTeam);
            ZeroGUI::Checkbox((char*)"FOV Circle", &fovc);

            ZeroGUI::Checkbox((char*)"Crosshair", &cross);
            
            ZeroGUI::Hotkey((char*)"Aimbot Key [need update]", FVector2D{ 80, 25 }, &AimbotKey);
            ZeroGUI::ColorPicker((char*)"Fov Circle Color", &Circlecolor);
            ZeroGUI::ColorPicker((char*)"Crosshair Color", &crosscolor);
        }

        if (tab == 2) {
            WindowSize = FVector2D{ 500.0f, 500.0f };
            ZeroGUI::Text((char*)"WP");
            ZeroGUI::Checkbox((char*)"Infinite Ammo", &InfiniteAmmo);
            
            ZeroGUI::Checkbox((char*)"Full Auto", &FullAuto);
            
            ZeroGUI::Checkbox((char*)"No Recoil", &NoRecoil);
            
            ZeroGUI::Checkbox((char*)"Insane Fire Rate", &FireRate);
            ZeroGUI::PushNextElementY(16.f);
            ZeroGUI::SliderFloat((char*)"Shots Per Burst", &ShotsPerBurst, 3.0f, 30.0f);
            ZeroGUI::Text((char*)"Nuker Hotkey & Settings");
            ZeroGUI::Hotkey((char*)"Nuker", FVector2D{ 80, 25 }, &NukerKey);
            
            ZeroGUI::Checkbox((char*)"Auto-Nuke", &AutoNuke);
            
            ZeroGUI::Checkbox((char*)"Ignore Team", &IgnoreTeamNuker);
        }

        if (tab == 1) {
            WindowSize = FVector2D{ 500.0f, 600.0f };
            ZeroGUI::Text((char*)"VS");
            ZeroGUI::Checkbox((char*)"Enable ESP", &EnableESP);
            
            ZeroGUI::Checkbox((char*)"Nametags", &Nametags);
            
            ZeroGUI::Checkbox((char*)"Boxes", &Boxes);
            
            ZeroGUI::Checkbox((char*)"Corner Box", &cornerbox);
            ZeroGUI::Checkbox((char*)"Snaplines", &Snaplines);
            ZeroGUI::PushNextElementY(16.0f);
            ZeroGUI::Checkbox((char*)"Ignore Team", &IgnoreTeamESP);
            
            ZeroGUI::Checkbox((char*)"Visible Colors", &VisibleColors);
            ZeroGUI::Checkbox((char*)"Visible Only", &VisibleOnly);
            ZeroGUI::SliderFloat((char*)"Snaplines X", &SnaplinesX, 0.f, horizontal, (char*)"%.0f");
            ZeroGUI::SliderFloat((char*)"Snaplines Y", &SnaplinesY, 0.f, vertical, (char*)"%.0f");
            ZeroGUI::PushNextElementY(12.0f);
            
            ZeroGUI::ColorPicker((char*)"Global Color", &ESPColor);
            ZeroGUI::ColorPicker((char*)"Team Color", &ESPTeamColor);
            ZeroGUI::ColorPicker((char*)"Enemy Color", &ESPEnemyColor);
            ZeroGUI::ColorPicker((char*)"Visible Color", &ESPVisibleColor);

        }

        if (tab == 3) {
            WindowSize = FVector2D{ 500.0f, 500.0f };
            ZeroGUI::Text((char*)"EX");
            ZeroGUI::Checkbox((char*)"Fly", &Fly);
            ZeroGUI::Checkbox((char*)"Night Vision ON", &nightmode);
            ZeroGUI::Checkbox((char*)"Night Vision OFF", &nightmode2);
            ZeroGUI::Checkbox((char*)"Night to Day", &daytime);
            ZeroGUI::Checkbox((char*)"Noclip", &Noclip);
            
            ZeroGUI::Checkbox((char*)"Desync", &Desync);
            ZeroGUI::SliderFloat((char*)"Speed", &Speed, 1.f, 5.f, (char*)"%.00f");
            
            ZeroGUI::SliderFloat((char*)"FOV", &FOV, 70.f, 160.f, (char*)"%.0f");
            ZeroGUI::Text((char*)"Menu Key & Discord Invite");
            ZeroGUI::Hotkey((char*)"Menu Key", FVector2D{ 80, 25 }, &MenuKey);
            
            if (ZeroGUI::Button((char*)"Unknowcheats", FVector2D{ 100, 25 })) {
                ShellExecute(NULL, NULL, "https://www.unknowncheats.me/forum/other-fps-games/574319-operation-harsh-doorstop-internal.html", 0, 0, SW_SHOW);
            }
            if (ZeroGUI::Button((char*)"Github", FVector2D{ 115, 25 })) {
                ShellExecute(NULL, NULL, "https://github.com/Buck3ts41/", 0, 0, SW_SHOW);
            }
            ZeroGUI::Text((char*)"Operation harsh doorstop have 0 anticheat, so have fun!", false, false);
        }
    }
    ZeroGUI::Render();
   
    
    UWorld** p_uworld = reinterpret_cast<UWorld**>(UWorld::GWorld);
    Nullcheck(p_uworld);
    Nullcheck(*p_uworld);
    UGameInstance* OwningGameInstance = (*p_uworld)->OwningGameInstance;
    Nullcheck(OwningGameInstance);
    auto GameState = (*p_uworld)->GameState;
    Nullcheck(GameState);
    UWorld* gworld = UWorld::GWorld[0];
    Nullcheck(gworld);
    auto persistent_level = gworld->PersistentLevel;
    Nullcheck(persistent_level);
    ULocalPlayer* localplayer = gworld->OwningGameInstance->LocalPlayers[0];
    Nullcheck(localplayer);
    APlayerController* local_controller = OwningGameInstance->LocalPlayers[0]->PlayerController;
    Nullcheck(local_controller);
    AHDPlayerCharacter* SelfPlayer = static_cast<AHDPlayerCharacter*>(local_controller->AcknowledgedPawn);
    Nullcheck(SelfPlayer);
    auto Character = reinterpret_cast<AHDPlayerCharacter*>(SelfPlayer);
    Nullcheck(Character);
    auto BaseWeapon = reinterpret_cast<AHDBaseWeapon*>(SelfPlayer->EquippedItem);
    Nullcheck(BaseWeapon);

    if (FullAuto) {
        NullcheckWeapon(SelfPlayer, BaseWeapon);
        auto Mode = EFireMode::Auto;
        if (BaseWeapon->SelectedFireMode != Mode) {
            BaseWeapon->SelectedFireMode = Mode;
        }
    }

    if (InfiniteAmmo) {
        NullcheckWeapon(SelfPlayer, BaseWeapon);
        BaseWeapon->bUsesAmmo = false;
        infAmmoCheck = true;
    }
    else {
        if (infAmmoCheck && !InfiniteAmmo) {
            NullcheckWeapon(SelfPlayer, BaseWeapon);
            BaseWeapon->bUsesAmmo = true;
            infAmmoCheck = true;
        }
    }

    if (NoRecoil) {
        NullcheckWeapon(SelfPlayer, BaseWeapon);
        if (BaseWeapon->bNoRecoil != NoRecoil) {
            BaseWeapon->bNoRecoil = true;
        }
    }

    if (FireRate) {
        NullcheckWeapon(SelfPlayer, BaseWeapon);
        if (BaseWeapon->FireRate != 0.0001f) {
            BaseWeapon->FireRate = 0.0001f;
        }
    }

    if (ShotsPerBurst > 3.0f) {
        NullcheckWeapon(SelfPlayer, BaseWeapon);
        BaseWeapon->ShotsPerBurst = ShotsPerBurst;
    }
    
    if (KeyPressed(NukerKey)) {
        Nuke();
    }

    if (AutoNuke) {
        Nuke();
    }
    
    if (Fly && Desync) {
        Desync = false;
    }

    if (Fly)
    {
        Character->SetReplicateMovement(false);
        Character->CharacterMovement->MovementMode = EMovementMode::MOVE_Flying; // set to fly mode so we don't fall down
        ResetFly = true;
    }
    else if (ResetFly)
    {
        Character->CharacterMovement->MovementMode = EMovementMode::MOVE_Falling;
        Character->SetReplicateMovement(true);
        ResetFly = false;
    }

    if (Noclip) {
        Character->bActorEnableCollision = false;
    }
    else {
        Character->bActorEnableCollision = true;
    }
    


    bool check = false;
    if (Desync && !Fly) {
        Character->SetReplicateMovement(false);
        check = true;
    } else if (check) {
        Character->SetReplicateMovement(true);
    }
    Nullcheck(local_controller);
    Nullcheck(local_controller->PlayerCameraManager);
    local_controller->PlayerCameraManager->DefaultFOV = FOV;
    Nullcheck(Character);
    Character->CustomTimeDilation = Speed;
}

inline void DrawLine(UCanvas* canvas, FVector2D Position1, FVector2D Position2, float Thickness, FLinearColor Color) {
    canvas->K2_DrawLine(Position1, Position2, Thickness, Color);
}

inline void DrawBox(UCanvas* canvas, FVector2D TopLeft, FVector2D DownRight, FLinearColor Color, float Thickness) {
    auto h = DownRight.Y - TopLeft.Y;
    auto w = DownRight.X - TopLeft.X;

    auto downleft = FVector2D{ TopLeft.X, DownRight.Y };
    auto topright = FVector2D{ DownRight.X, TopLeft.Y };

    DrawLine(canvas, TopLeft, { TopLeft.X, TopLeft.Y +h * 1 }, Thickness, Color);
    DrawLine(canvas, TopLeft, { TopLeft.X + w * 1, TopLeft.Y}, Thickness, Color);

    DrawLine(canvas, DownRight, { DownRight.X, DownRight.Y - h * 1 }, Thickness, Color);
    DrawLine(canvas, DownRight, { DownRight.X - w * 1, DownRight.Y }, Thickness, Color);

    DrawLine(canvas, downleft, { downleft.X, downleft.Y - h * 1 }, Thickness, Color);
    DrawLine(canvas, downleft, { downleft.X + w * 1, downleft.Y }, Thickness, Color);

    DrawLine(canvas, topright, { topright.X, topright.Y + h * 1 }, Thickness, Color);
    DrawLine(canvas, topright, { topright.X - w * 1, topright.Y }, Thickness, Color);
}
inline void DrawBoxCorners(UCanvas* canvas, FVector2D TopLeft, FVector2D DownRight, FLinearColor Color, float Thickness)
{
    auto downleft = FVector2D{ TopLeft.X, DownRight.Y };
    auto topright = FVector2D{ DownRight.X, TopLeft.Y };

    // Top left corner
    DrawLine(canvas, TopLeft, { TopLeft.X + Thickness, TopLeft.Y }, Thickness, Color);
    DrawLine(canvas, TopLeft, { TopLeft.X, TopLeft.Y + Thickness }, Thickness, Color);

    // Top right corner
    DrawLine(canvas, { topright.X - Thickness, topright.Y }, topright, Thickness, Color);
    DrawLine(canvas, { topright.X, topright.Y + Thickness }, topright, Thickness, Color);

    // Bottom right corner
    DrawLine(canvas, DownRight, { DownRight.X - Thickness, DownRight.Y }, Thickness, Color);
    DrawLine(canvas, DownRight, { DownRight.X, DownRight.Y - Thickness }, Thickness, Color);

    // Bottom left corner
    DrawLine(canvas, { downleft.X + Thickness, downleft.Y }, downleft, Thickness, Color);
    DrawLine(canvas, { downleft.X, downleft.Y - Thickness }, downleft, Thickness, Color);
}


FVector inline VectorSubtract(FVector Vector1, FVector Vector2)
{
    FVector Vec;
    Vec.X = Vector1.X - Vector2.X;
    Vec.Y = Vector1.Y - Vector2.Y;
    Vec.Z = Vector1.Z - Vector2.Z;
    return Vec;
}

FVector inline VectorAdd(FVector Vector1, FVector Vector2)
{
    FVector Vec;
    Vec.X = Vector1.X + Vector2.X;
    Vec.Y = Vector1.Y + Vector2.Y;
    Vec.Z = Vector1.Z + Vector2.Z;
    return Vec;
}

inline void SetRotation(UCanvas* Canvas, AHDPlayerCharacter* Target, FVector AimLocation, bool is_visible)
{
    UWorld** p_uworld = reinterpret_cast<UWorld**>(UWorld::GWorld);
    Nullcheck(p_uworld);
    Nullcheck(*p_uworld);
    UGameInstance* OwningGameInstance = (*p_uworld)->OwningGameInstance;
    Nullcheck(OwningGameInstance);
    auto GameState = (*p_uworld)->GameState;
    Nullcheck(GameState);
    UWorld* gworld = UWorld::GWorld[0];
    Nullcheck(gworld);
    ULocalPlayer* localplayer = gworld->OwningGameInstance->LocalPlayers[0];
    Nullcheck(localplayer);
    auto Camera = localplayer->PlayerController->PlayerCameraManager;
    FRotator AimRotation;
    //	FRotator AimRotation_Temp;
    FVector  MyLocation = FVector();
    FRotator MyRotation = FRotator();
    FVector Velocity = FVector();
    FVector Difference = FVector();

    if (!Target || !localplayer->PlayerController || !Camera) return;

    // Get player location and rotator
    MyLocation = Camera->GetCameraLocation();
    MyRotation = Camera->GetCameraRotation();

    // Check angles
    //float ViewAngle = GetViewAngle(Canvas, Target->Location, MyLocation, MyRotation);
    //if (!CheckAngle(ViewAngle, gUE.gPlayerController->FOVAngle / 5.0f)) return;

    // Check visibility
    if (AimbotVisibleOnly && !is_visible) { return; }

    // Accuracy adjustments
    float fDistance;
    auto gMath = (UKismetMathLibrary*)UKismetMathLibrary::StaticClass();
    Difference = VectorSubtract(AimLocation, MyLocation);
    fDistance = gMath->STATIC_VSize(Difference);
    float TimeScale = 0.0193f;
    Nullcheck(localplayer);
    Nullcheck(localplayer->PlayerController);
    Nullcheck(localplayer->PlayerController->Pawn);
    Nullcheck(localplayer->PlayerController->Pawn->PlayerState);
    short Ping = localplayer->PlayerController->Pawn->PlayerState->Ping * 4;
    float ScaleValue = (float)(Ping)+(float)(fDistance * TimeScale); // most bullets here are just projectiles, so do a bit of velocity tweaking
    if (ScaleValue < 1.0f) ScaleValue = 1.0;
    float fVelocity;
    Velocity = Target->GetVelocity();
    fVelocity = gMath->STATIC_VSize(Velocity);
    if (fVelocity > 150.0f)
    {
        Velocity.X *= ScaleValue / fVelocity; Velocity.Y *= ScaleValue / fVelocity; Velocity.Z *= ScaleValue / fVelocity;
        AimLocation = VectorAdd(AimLocation, Velocity);
    }

    AimRotation = gMath->STATIC_FindLookAtRotation(MyLocation, AimLocation);

    //gUE.gPlayerController->PlayerCameraManager->ViewTarget.POV.Rotation = AimRotation;
    localplayer->PlayerController->ClientSetRotation(AimRotation, false);
}

typedef void(__thiscall* post_render_type)(UGameViewportClient*, UCanvas*);
post_render_type original_post_render = nullptr;
void posthook(UGameViewportClient* vp_client, UCanvas* canvas)
{
    try
    {
        GetDesktopResolution(horizontal, vertical);
        canvas->K2_DrawText(get_font(), FString(L"[Buck3ts41 Internal]\nDefault Menu Key: INSERT\nKingCU.cc"), FVector2D(10, 15), FVector2D(0.5f, 0.5f), rgb2rgbfl(252, 232, 3), 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
        ZeroGUI::SetupCanvas(canvas);
        Tick();
        UWorld** p_uworld = reinterpret_cast<UWorld**>(UWorld::GWorld);
        Nullcheck(p_uworld);
        Nullcheck(*p_uworld);
        UGameInstance* OwningGameInstance = (*p_uworld)->OwningGameInstance;
        Nullcheck(OwningGameInstance);
        auto GameState = (*p_uworld)->GameState;
        Nullcheck(GameState);
        UWorld* gworld = UWorld::GWorld[0];
        Nullcheck(gworld);
        ULocalPlayer* localplayer = gworld->OwningGameInstance->LocalPlayers[0];
        Nullcheck(localplayer);
        if (fovc) {
            DrawCircle(FVector2D(960, 540), 160, 32, Circlecolor);

        }
        if (cross) {
            FVector2D screenMiddle = FVector2D(960, 540);

            // Draw the vertical line of the crosshair
            FVector2D vStart = screenMiddle + FVector2D(0, -20 / 2);
            FVector2D vEnd = screenMiddle + FVector2D(0, 20 / 2);
            Draw_Line(vStart, vEnd, 2, crosscolor);

            // Draw the horizontal line of the crosshair
            FVector2D hStart = screenMiddle + FVector2D(-20 / 2, 0);
            FVector2D hEnd = screenMiddle + FVector2D(20 / 2, 0);
            Draw_Line(hStart, hEnd, 2, crosscolor);
            
        }
        bool nigers = false;
        if (nightmode and nigers == false) {
            nightON();
            nigers == true;
        }
        if (nightmode2 and nigers == true) {
            nightOFF();
            nigers == false;
        }

        if (daytime) {
            tymeday();
        }
        
        if (canvas && EnableESP) {
            auto pArray = GameState->PlayerArray;
            if (pArray.Count() > 1) {
                for (USHORT i = 0; i < pArray.Count(); i++) {
                    auto& ent = pArray[i];
                    if (ent != localplayer->PlayerController->PlayerState) {
                        auto Character = static_cast<AHDPlayerCharacter*>(ent->PawnPrivate);
                        NullcheckC(Character);
                        NullcheckC(ent->PawnPrivate);
                        if (Character->Health <= 0) continue;
                        auto Location = Character->ReplicatedMovement.Location;
                        bool is_visible = localplayer->PlayerController->LineOfSightTo(Character, { 0.f, 0.f, 0.f }, false);
                        FVector2D screen;
                        localplayer->PlayerController->ProjectWorldLocationToScreen(Location, &screen, NULL);
                        AHDPlayerCharacter* SelfPlayer = static_cast<AHDPlayerCharacter*>(localplayer->PlayerController->AcknowledgedPawn);
                        NullcheckC(SelfPlayer);
                        if (Snaplines) {
                            if (IgnoreTeamESP) {
                                if (Character->TeamNum != SelfPlayer->TeamNum) {
                                    if (VisibleColors) {
                                        if (VisibleOnly && is_visible) {
                                            canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPVisibleColor);
                                        }
                                        else if (!VisibleOnly) {
                                            if (is_visible) {
                                                canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPVisibleColor);
                                            }
                                            else {
                                                canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPColor);
                                            }
                                        }
                                    }
                                    else {
                                        canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPColor);
                                    }
                                }
                            }
                            else {
                                if (Character->TeamNum != SelfPlayer->TeamNum) {
                                    if (VisibleColors) {
                                        if (VisibleOnly && is_visible) {
                                            canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPVisibleColor);
                                        }
                                        else if (!VisibleOnly) {
                                            if (is_visible) {
                                                canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPVisibleColor);
                                            }
                                            else {
                                                canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPEnemyColor);
                                            }
                                        }
                                    }
                                    else {
                                        canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPEnemyColor);
                                    }
                                }
                                else {
                                    if (VisibleColors) {
                                        if (VisibleOnly && is_visible) {
                                            canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPVisibleColor);
                                        }
                                        else if (!VisibleOnly) {
                                            if (is_visible) {
                                                canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPVisibleColor);
                                            }
                                            else {
                                                canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPTeamColor);
                                            }
                                        }
                                    }
                                    else {
                                        canvas->K2_DrawLine(FVector2D(SnaplinesX, SnaplinesY), FVector2D(screen.X, screen.Y - 10), 1, ESPTeamColor);
                                    }
                                }
                            }
                        }
                        FString Player_Name = FString((Character->PlayerState->GetPlayerName().wc_str() + std::wstring(L" [") + std::to_wstring((int)(localplayer->PlayerController->GetDistanceTo(Character) / 100)) + L"m]").c_str());
                        
                        FVector2D NametagPosition = FVector2D(screen.X - 12.f, screen.Y - 22.f);
                        if (Nametags) {
                            if (IgnoreTeamESP) {
                                if (Character->TeamNum != SelfPlayer->TeamNum) {
                                    if (VisibleColors) {
                                        if (VisibleOnly && is_visible) {
                                            canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPVisibleColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                        }
                                        else if (!VisibleOnly) {
                                            if (is_visible) {
                                                canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPVisibleColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                            }
                                            else {
                                                canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                            }
                                        }
                                    }
                                    else {
                                        canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                    }
                                    
                                }
                            }
                            else {
                                if (Character->TeamNum != SelfPlayer->TeamNum) {
                                    if (VisibleColors) {
                                        if (VisibleOnly && is_visible) {
                                            canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPVisibleColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                        }
                                        else if (!VisibleOnly) {
                                            if (is_visible) {
                                                canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPVisibleColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                            }
                                            else {
                                                canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPEnemyColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                            }
                                        }
                                    }
                                    else {
                                        canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPEnemyColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                    }
                                }
                                else {
                                    if (VisibleColors) {
                                        if (VisibleOnly && is_visible) {
                                            canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPVisibleColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                        }
                                        else if (!VisibleOnly) {
                                            if (is_visible) {
                                                canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPVisibleColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                            }
                                            else {
                                                canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPTeamColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                            }
                                        }
                                    }
                                    else {
                                        canvas->K2_DrawText(get_font(), Player_Name, NametagPosition, FVector2D(0.45f, 0.45f), ESPTeamColor, 1.0f, FLinearColor(0, 0, 0, 1), FVector2D(), false, true, true, FLinearColor(0, 0, 0, 1));
                                    }
                                }
                            }
                        }

                        if (Boxes) {
                            if (IgnoreTeamESP) {
                                if (Character->TeamNum != SelfPlayer->TeamNum) {
                                    FVector ActorLoc;
                                    FVector BoxExtent;

                                    Character->GetActorBounds(true, &ActorLoc, &BoxExtent, false);
                                    auto rotation = Character->K2_GetActorRotation();

                                    FVector foot_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z + (BoxExtent.Z) };
                                    FVector head_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z - (BoxExtent.Z) };

                                    FVector2D head_pos, foot_pos;

                                    if (localplayer->PlayerController->ProjectWorldLocationToScreen(foot_location, &foot_pos, false) && localplayer->PlayerController->ProjectWorldLocationToScreen(head_location, &head_pos, false)) {
                                        FVector2D w2sRes;
                                        if (localplayer->PlayerController->ProjectWorldLocationToScreen(ActorLoc, &w2sRes, false)) {
                                            const float h = abs(foot_pos.Y - head_pos.Y);
                                            const float w = h * 0.6f;

                                            FVector2D top = { head_pos.X - w * 0.5f, head_pos.Y };
                                            FVector2D bottom = { head_pos.X + w * 0.5f, foot_pos.Y };

                                            if (VisibleColors) {
                                                if (VisibleOnly && is_visible) {
                                                    DrawBox(canvas, top, bottom, ESPVisibleColor, 1.f);
                                                }
                                                else if (!VisibleOnly) {
                                                    if (is_visible) {
                                                        DrawBox(canvas, top, bottom, ESPVisibleColor, 1.f);
                                                    }
                                                    else {
                                                        DrawBox(canvas, top, bottom, ESPColor, 1.f);
                                                    }
                                                }
                                            }
                                            else {
                                                DrawBox(canvas, top, bottom, ESPColor, 1.f);
                                            }
                                        }
                                    }
                                }
                            }
                            else {
                                if (Character->TeamNum != SelfPlayer->TeamNum) {
                                    FVector ActorLoc;
                                    FVector BoxExtent;

                                    Character->GetActorBounds(true, &ActorLoc, &BoxExtent, false);
                                    auto rotation = Character->K2_GetActorRotation();

                                    FVector foot_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z + (BoxExtent.Z) };
                                    FVector head_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z - (BoxExtent.Z) };

                                    FVector2D head_pos, foot_pos;

                                    if (localplayer->PlayerController->ProjectWorldLocationToScreen(foot_location, &foot_pos, false) && localplayer->PlayerController->ProjectWorldLocationToScreen(head_location, &head_pos, false)) {
                                        FVector2D w2sRes;
                                        if (localplayer->PlayerController->ProjectWorldLocationToScreen(ActorLoc, &w2sRes, false)) {
                                            const float h = abs(foot_pos.Y - head_pos.Y);
                                            const float w = h * 0.6f;

                                            FVector2D top = { head_pos.X - w * 0.5f, head_pos.Y };
                                            FVector2D bottom = { head_pos.X + w * 0.5f, foot_pos.Y };
                                            if (VisibleColors) {
                                                if (VisibleOnly && is_visible) {
                                                    DrawBox(canvas, top, bottom, ESPVisibleColor, 1.f);
                                                }
                                                else if (!VisibleOnly) {
                                                    if (is_visible) {
                                                        DrawBox(canvas, top, bottom, ESPVisibleColor, 1.f);
                                                    }
                                                    else {
                                                        DrawBox(canvas, top, bottom, ESPEnemyColor, 1.f);
                                                    }
                                                }
                                            }
                                            else {
                                                DrawBox(canvas, top, bottom, ESPEnemyColor, 1.f);
                                            }
                                        }
                                    }
                                }
                                else {
                                    FVector ActorLoc;
                                    FVector BoxExtent;

                                    Character->GetActorBounds(true, &ActorLoc, &BoxExtent, false);
                                    auto rotation = Character->K2_GetActorRotation();

                                    FVector foot_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z + (BoxExtent.Z) };
                                    FVector head_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z - (BoxExtent.Z) };

                                    FVector2D head_pos, foot_pos;

                                    if (localplayer->PlayerController->ProjectWorldLocationToScreen(foot_location, &foot_pos, false) && localplayer->PlayerController->ProjectWorldLocationToScreen(head_location, &head_pos, false)) {
                                        FVector2D w2sRes;
                                        if (localplayer->PlayerController->ProjectWorldLocationToScreen(ActorLoc, &w2sRes, false)) {
                                            const float h = abs(foot_pos.Y - head_pos.Y);
                                            const float w = h * 0.6f;

                                            FVector2D top = { head_pos.X - w * 0.5f, head_pos.Y };
                                            FVector2D bottom = { head_pos.X + w * 0.5f, foot_pos.Y };
                                            if (VisibleColors) {
                                                if (VisibleOnly && is_visible) {
                                                    DrawBox(canvas, top, bottom, ESPVisibleColor, 1.f);
                                                }
                                                else if (!VisibleOnly) {
                                                    if (is_visible) {
                                                        DrawBox(canvas, top, bottom, ESPVisibleColor, 1.f);
                                                    }
                                                    else {
                                                        DrawBox(canvas, top, bottom, ESPTeamColor, 1.f);
                                                    }
                                                }
                                            }
                                            else {
                                                DrawBox(canvas, top, bottom, ESPTeamColor, 1.f);
                                            }
                                        }
                                    }
                                }
                            }
                           
                        }
                        if (cornerbox) {
                            if (IgnoreTeamESP) {
                                if (Character->TeamNum != SelfPlayer->TeamNum) {
                                    FVector ActorLoc;
                                    FVector BoxExtent;

                                    Character->GetActorBounds(true, &ActorLoc, &BoxExtent, false);
                                    auto rotation = Character->K2_GetActorRotation();

                                    FVector foot_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z + (BoxExtent.Z) };
                                    FVector head_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z - (BoxExtent.Z) };

                                    FVector2D head_pos, foot_pos;

                                    if (localplayer->PlayerController->ProjectWorldLocationToScreen(foot_location, &foot_pos, false) && localplayer->PlayerController->ProjectWorldLocationToScreen(head_location, &head_pos, false)) {
                                        FVector2D w2sRes;
                                        if (localplayer->PlayerController->ProjectWorldLocationToScreen(ActorLoc, &w2sRes, false)) {
                                            const float h = abs(foot_pos.Y - head_pos.Y) * 0.75f;
                                            const float w = h * 0.60f;

                                            FVector2D top = { head_pos.X - w * 0.5f, head_pos.Y };
                                            FVector2D bottom = { head_pos.X + w * 0.5f, foot_pos.Y };

                                            if (VisibleColors) {
                                                if (VisibleOnly && is_visible) {
                                                    DrawBoxCorners(canvas, top, bottom, ESPVisibleColor, 1.8f);
                                                }
                                                else if (!VisibleOnly) {
                                                    if (is_visible) {
                                                        DrawBoxCorners(canvas, top, bottom, ESPVisibleColor, 1.8f);
                                                    }
                                                    else {
                                                        DrawBoxCorners(canvas, top, bottom, ESPColor, 1.8f);
                                                    }
                                                }
                                            }
                                            else {
                                                DrawBoxCorners(canvas, top, bottom, ESPColor, 1.8f);
                                            }
                                        }
                                    }
                                }
                            }
                            else {
                                if (Character->TeamNum != SelfPlayer->TeamNum) {
                                    FVector ActorLoc;
                                    FVector BoxExtent;

                                    Character->GetActorBounds(true, &ActorLoc, &BoxExtent, false);
                                    auto rotation = Character->K2_GetActorRotation();

                                    FVector foot_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z + (BoxExtent.Z) };
                                    FVector head_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z - (BoxExtent.Z) };

                                    FVector2D head_pos, foot_pos;

                                    if (localplayer->PlayerController->ProjectWorldLocationToScreen(foot_location, &foot_pos, false) && localplayer->PlayerController->ProjectWorldLocationToScreen(head_location, &head_pos, false)) {
                                        FVector2D w2sRes;
                                        if (localplayer->PlayerController->ProjectWorldLocationToScreen(ActorLoc, &w2sRes, false)) {
                                            const float h = abs(foot_pos.Y - head_pos.Y) * 0.75f;
                                            const float w = h * 0.60f;

                                            FVector2D top = { head_pos.X - w * 0.5f, head_pos.Y };
                                            FVector2D bottom = { head_pos.X + w * 0.5f, foot_pos.Y };
                                            if (VisibleColors) {
                                                if (VisibleOnly && is_visible) {
                                                    DrawBoxCorners(canvas, top, bottom, ESPVisibleColor, 1.8f);
                                                }
                                                else if (!VisibleOnly) {
                                                    if (is_visible) {
                                                        DrawBoxCorners(canvas, top, bottom, ESPVisibleColor, 1.8f);
                                                    }
                                                    else {
                                                        DrawBoxCorners(canvas, top, bottom, ESPEnemyColor, 1.8f);
                                                    }
                                                }
                                            }
                                            else {
                                                DrawBoxCorners(canvas, top, bottom, ESPEnemyColor, 1.8f);
                                            }
                                        }
                                    }
                                }
                                else {
                                    FVector ActorLoc;
                                    FVector BoxExtent;

                                    Character->GetActorBounds(true, &ActorLoc, &BoxExtent, false);
                                    auto rotation = Character->K2_GetActorRotation();

                                    FVector foot_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z + (BoxExtent.Z) };
                                    FVector head_location = { ActorLoc.X , ActorLoc.Y, ActorLoc.Z - (BoxExtent.Z) };

                                    FVector2D head_pos, foot_pos;

                                    if (localplayer->PlayerController->ProjectWorldLocationToScreen(foot_location, &foot_pos, false) && localplayer->PlayerController->ProjectWorldLocationToScreen(head_location, &head_pos, false)) {
                                        FVector2D w2sRes;
                                        if (localplayer->PlayerController->ProjectWorldLocationToScreen(ActorLoc, &w2sRes, false)) {
                                            const float h = abs(foot_pos.Y - head_pos.Y) * 0.75f;
                                            const float w = h * 0.60f;

                                            FVector2D top = { head_pos.X - w * 0.5f, head_pos.Y };
                                            FVector2D bottom = { head_pos.X + w * 0.5f, foot_pos.Y };
                                            if (VisibleColors) {
                                                if (VisibleOnly && is_visible) {
                                                    DrawBoxCorners(canvas, top, bottom, ESPVisibleColor, 1.8f);
                                                }
                                                else if (!VisibleOnly) {
                                                    if (is_visible) {
                                                        DrawBoxCorners(canvas, top, bottom, ESPVisibleColor, 1.8f);
                                                    }
                                                    else {
                                                        DrawBoxCorners(canvas, top, bottom, ESPTeamColor, 1.8f);
                                                    }
                                                }
                                            }
                                            else {
                                                DrawBoxCorners(canvas, top, bottom, ESPTeamColor, 1.8f);
                                            }
                                        }
                                    }
                                }
                            }

                        }
                    }
                }
            }
            
        }

        if (EnableAimbot) {
            AHDPlayerCharacter* SelfPlayer = static_cast<AHDPlayerCharacter*>(localplayer->PlayerController->AcknowledgedPawn);
            auto pArray = GameState->PlayerArray;
            if (pArray.Count() > 1) {
                for (USHORT i = 0; i < pArray.Count(); i++) {
                    auto& ent = pArray[i];
                    NullcheckC(SelfPlayer);
                    if (ent != localplayer->PlayerController->PlayerState) {
                        auto Character = static_cast<AHDPlayerCharacter*>(ent->PawnPrivate);
                        NullcheckC(Character);
                        NullcheckC(ent->PawnPrivate);
                        if (Character->Health <= 0) continue;
                        if (AimbotTargetTeam) {
                            auto Location = Character->ReplicatedMovement.Location;
                            bool is_visible = localplayer->PlayerController->LineOfSightTo(Character, { 0.f, 0.f, 0.f }, false);
                            FVector2D screen;
                            localplayer->PlayerController->ProjectWorldLocationToScreen(Location, &screen, NULL);
                            AHDPlayerCharacter* SelfPlayer = static_cast<AHDPlayerCharacter*>(localplayer->PlayerController->AcknowledgedPawn);
                            NullcheckC(SelfPlayer);
                            FVector ActorLoc = Character->ReplicatedMovement.Location;
                            if (KeyPressed(AimbotKey)) {
                                SetRotation(canvas, Character, ActorLoc, is_visible);
                            }
                        }
                        else {
                            if (Character->TeamNum != SelfPlayer->TeamNum) {
                                auto Location = Character->ReplicatedMovement.Location;
                                bool is_visible = localplayer->PlayerController->LineOfSightTo(Character, { 0.f, 0.f, 0.f }, false);
                                FVector2D screen;
                                localplayer->PlayerController->ProjectWorldLocationToScreen(Location, &screen, NULL);
                                AHDPlayerCharacter* SelfPlayer = static_cast<AHDPlayerCharacter*>(localplayer->PlayerController->AcknowledgedPawn);
                                NullcheckC(SelfPlayer);
                                FVector ActorLoc = Character->ReplicatedMovement.Location;
                                if (KeyPressed(AimbotKey)) {
                                    SetRotation(canvas, Character, ActorLoc, is_visible);
                                }
                            }
                        }
                        
                    }
                }
            }
        }
        
    }
    catch (std::exception& e)
    {
        return original_post_render(vp_client, canvas);
    }
    return original_post_render(vp_client, canvas);
}

void InitOHD() {
    InitSdk(L"HarshDoorstop-Win64-Shipping.exe", 0x41E4D88, 0x41CC340, 0x4327088);
    UWorld* uworld = *UWorld::GWorld;
    Nullcheck(uworld);
    
    auto owninggameinstance = uworld->OwningGameInstance;
    Nullcheck(owninggameinstance);

    ULocalPlayer* localplayer = owninggameinstance->LocalPlayers[0];
    Nullcheck(localplayer);

    auto game_vpc = localplayer->ViewportClient;
    void** game_vpc_vtable = *reinterpret_cast<void***>(game_vpc);

    DWORD OldProtection;

    VirtualProtect(&game_vpc_vtable[97], 8, PAGE_EXECUTE_READWRITE, &OldProtection);
    original_post_render = decltype(original_post_render)(game_vpc_vtable[97]);
    game_vpc_vtable[97] = posthook;
    VirtualProtect(&game_vpc_vtable[97], 8, OldProtection, &OldProtection);

    return;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        InitOHD();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

