#include "core/Application.h"


void MisigmaApp::Initialize()
{
    wi::Application::Initialize();

    wi::renderer::SetShaderSourcePath(wi::helper::GetCurrentPath() + "/shaders/");


    loadingState  = std::make_unique<LoadingState>();
    mainMenuState = std::make_unique<MainMenuState>();
    gameplayState = std::make_unique<GameplayState>();
    optionsState  = std::make_unique<OptionsState>();
    lobbyState    = std::make_unique<LobbyState>();

    
    mainMenuState->Init(this);
    gameplayState->Init(this);
    optionsState->Init(this, gameplayState.get());
    lobbyState->Init(this);

    
    loadingState->Init(this);
    ActivatePath(loadingState.get(), 0.0f);

    currentState = GameStateID::Loading;
}


void MisigmaApp::GoToMainMenu(float fadeSec)
{
    
    mainMenuState->Load();
    ActivatePath(mainMenuState.get(), fadeSec);
    currentState = GameStateID::MainMenu;
}


void MisigmaApp::StartGame(float fadeSec)
{
    gameplayState->Load();
    ActivatePath(gameplayState.get(), fadeSec);
    currentState = GameStateID::Gameplay;
}


void MisigmaApp::GoToOptions(float fadeSec)
{
    optionsState->Load();
    ActivatePath(optionsState.get(), fadeSec);
    currentState = GameStateID::Options;
}


void MisigmaApp::GoToLobby(float fadeSec)
{
    lobbyState->Load();
    ActivatePath(lobbyState.get(), fadeSec);
    currentState = GameStateID::Lobby;
}

