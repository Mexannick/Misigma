#pragma once
#include "WickedEngine.h"
#include "core/GameState.h"


#include "states/LoadingState.h"
#include "states/MainMenuState.h"
#include "states/GameplayState.h"
#include "states/OptionsState.h"
#include "states/LobbyState.h"







class MisigmaApp : public wi::Application
{
public:
    
    void Initialize() override;

    
    void GoToMainMenu(float fadeSec = 0.5f);
    void StartGame   (float fadeSec = 0.5f);
    void GoToOptions (float fadeSec = 0.4f);
    void GoToLobby   (float fadeSec = 0.4f);

    
    GameStateID GetCurrentState() const { return currentState; }

private:
    std::unique_ptr<LoadingState>   loadingState;
    std::unique_ptr<MainMenuState>  mainMenuState;
    std::unique_ptr<GameplayState>  gameplayState;
    std::unique_ptr<OptionsState>   optionsState;
    std::unique_ptr<LobbyState>     lobbyState;

    GameStateID currentState = GameStateID::Loading;
};
