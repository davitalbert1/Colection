// Ponto de entrada mínimo — toda a lógica está em App/UI/serviços (Passo 4, Regra 4)
#include "App.h"

int main() {
    App app;
    if (!app.initialize()) return 1;
    app.run();
    app.shutdown();
    return 0;
}
