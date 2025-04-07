#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define MENU    0 //Codigo de menu para variavel de opcao
#define LEITURA 1 //Primeiro modo de uso da funcao abre_scores
#define ATUALIZA 2 //Segundo modo de uso da funcao abre_scores
#define VIDA    3 //Vida maxima
#define LINHA   5 //Numero maximo de linhas de blocos em uma fase
#define COLUNA  10 //Numero maximo de colunas de blocos em uma fases

//Struct do jogador, com sua posicao, tamanho e vida
typedef struct{
    int vida;
    int pontos;
    Vector2 posic;
    Vector2 tam;
} Player;

/*Struct da bola, com sua posicao, velocidade, raio e uma variavel booleana (verdadeiro ou falso)
que indica se ela esta ativa ou nao*/
typedef struct{
    Vector2 posic;
    Vector2 vel;
    int raio;
    bool isiton;
}Bola;

/*Struct de blocos, a qual deve virar uma matriz, com a posicao e o booleano de cada bloco, que
indica se este esta ou nao ativo*/
typedef struct{
    Vector2 posic;
    bool isiton;
}Bloco;

//Struct reservada para guardar na memoria do programa os nomes dos 5 melhores jogadores
typedef struct{
    char nome[12];
    int pontos;
}Scores;

//Define game over (fara sentido mais adiante)
static const int screenW = 800; //Define largura de tela
static const int screenH = 450; //Define altura de tela
static const int select = 30; //Distancia de seleção
static const int xbloco = 60; //Tamanho horizontal do tijolo
static const int ybloco = 10; //Tamanho vertical do tijolo
static int opt = MENU; //Variavel global de menu
static bool gameOver = true;//Essa variavel booleana (falso ou verdadeiro) serve para indicar que o jogo acabou. Mais detalhes abaixo
static Player player = {0};//Cria e inicializa como zero todos os dados do player
static Bola bola = {0};//Cria e inicializa como zero todos os dados da bola
static Bloco bloco[LINHA][COLUNA] = {0};//Cria e inicializa como zero todos os dados da matriz de blocos
static Scores scores[5] = {0};//Cria e inicializa como nulo o vetor dos 5 melhores

void abre_scores(int modo){
    int i=0, flag = 0, j=0;
    FILE *arq;
    Scores default_players[5] = {{"DavMustaine", 500}, {"Slash", 400}, {"Ozzy", 300}, {"Brazino", 200}, {"Emmy White", 100}};
    //Default_players sao jogadores padroes feitos para serem inicializados caso o jogo nunca tenha sido jogado

    /*Neste momento, a funcao abre um arquivo binario em modo de leitura e, caso nao exista, fecha e abre novamente em modo
    de criacao e escrita para escrever os dados do vetor de jogadores padroes.
    */
    switch(modo){
    case 1:
            do{
                arq = fopen("highscores.bin", "rb");
                if(arq == NULL){
                    flag = 1;
                    fclose(arq);
                    arq = fopen("highscores.bin", "wb");

                    for(i=0; i < 5; i++){
                        fwrite(&default_players[i], sizeof(Scores), 1, arq);
                    }

                    fclose(arq);
                }else{
                    flag = 0;
                    for(i=0; i < 5; i++)
                        fread(&scores[i], sizeof(Scores), 1, arq);

                    fclose(arq);
                }
            }while(flag != 0);
            break;
    case 2:
            arq = fopen("highscores.bin", "wb");
            for(i=0; i < 5; i++)
                fwrite(&scores[i], sizeof(Scores), 1, arq);
            fclose(arq);
            break;
    }

        return;
    }
//Funcao que abre um arquivo .txt com as infos do mapa em letras que representam as cores ou sinal de hifen que representa espaco vazio
void abre_mapa(char nivel[5][10], int fase){
    int i, j;
    FILE *arq;
    char bloco;
    char arqname[11] = {'n', 'i', 'v', 'e', 'l', 'x', '.', 't', 'x', 't'};//Inicializa o nome com um x no lugar do numero que indica a fase.

    //Nomeia o arquivo a ser aberto dependendo do inteiro "fase"
    arqname[5] = fase + '0';

    arq = fopen(arqname, "r");

    /*Caso o arquivo nao exista, a funcao vai direto para um arquivo padrao chamado "nivelx.txt"
    que pode ser a fase personalizada por quem desejar (por exemplo o professor)*/
    if(arq == NULL){
        fclose(arq);
        arqname[5] = 'x';
        arq = fopen(arqname, "r");
    }

    //Traduz para a matriz nivel os caracteres do arquivo .txt
    for(i=0; i < LINHA; i++)
        for(j=0; j < COLUNA; j++){
            fscanf(arq, "%c", &bloco);
            if(bloco == '\n')
                fscanf(arq, "%c", &bloco);
            nivel[i][j] = bloco;
    }

    fclose(arq);

    return;
}

//Funcao que inicia as variaveis do jogo para que ele inicie
void start(char nivel[5][10], int fase){
    int i, j, posx, posy;

    //O if serve para testar se esta funcao nao foi aberta como retorno ao menu. Fara mais sentido abaixo.
    if(fase != MENU)
        abre_mapa(nivel, fase);

    /*Apenas acontece em caso de fim de jogo verdadeiro, por isso a variavel foi iniciada como verdadeiro.
    Tambem pode ser o caso da funcao ter sido chamada como retorno ao menu, que sera visto abaixo.
    */
    if(gameOver == true || fase == MENU){
        //Define valores do player
        player.posic = (Vector2){screenW/2, screenH*7/8};
        player.tam = (Vector2){screenW/10, 20};
        player.vida = VIDA;
        player.pontos = 0;

        //Define valores da bola
        bola.posic = (Vector2){screenW/2, screenH*7/8 - 30};
        bola.vel = (Vector2){0, 0};
        bola.raio = 8;
        bola.isiton = false;

        gameOver = false;
    }

    //Define valores dos blocos
    for(i=0; i < LINHA; i++)
            for(j=0; j < COLUNA; j++){
                if(nivel[i][j] != '-'){
                    bloco[i][j].isiton = true;
                    posx = (j * (xbloco + 8)) + 60;
                    posy = (i * (ybloco + 8)) + 8;
                    bloco[i][j].posic = (Vector2){posx, posy};
                }else
                    bloco[i][j].isiton = false;
            }

    return;
}
//Funcao que desenha cada frame do jogo de acordo com as posicoes instantaneas
void traduz_mapa(char nivel[5][10]){
    int i, j, posx, posy;
    char cor;

        BeginDrawing();
            ClearBackground(BLACK);

            //Desenha o player
            DrawRectangle(player.posic.x - player.tam.x/2, player.posic.y - player.tam.y/2, player.tam.x, player.tam.y, RAYWHITE);

            //Desenha as vidas
            if(player.vida >= 1)
                DrawRectangle(screenW - (xbloco + 8), screenH - 8 - ybloco/2, xbloco, ybloco, PINK);
            if(player.vida >= 2)
                DrawRectangle(screenW - 2*(xbloco + 8), screenH - 8 - ybloco/2, xbloco, ybloco, PINK);
            if(player.vida == VIDA)
                DrawRectangle(screenW - 3*(xbloco + 8), screenH - 8 - ybloco/2, xbloco, ybloco, PINK);


            //Desenha os pontos
            DrawText("SCORE: ", 10, screenH - 30, 30, WHITE);
            DrawText(TextFormat("%i", player.pontos), MeasureText("SCORE: ", 30) + 10, screenH - 30, 30, WHITE);


            //Desenha a bola
            DrawCircleV(bola.posic, bola.raio, MAROON);

            //Desenha os blocos dependendo da letra, a qual representa uma cor
            for(i=0; i < LINHA; i++)
                for(j=0; j < COLUNA; j++){
                    cor = tolower(nivel[i][j]);
                    posx = (j * (xbloco + 8)) + 60;
                    posy = (i * (ybloco + 8)) + 8;
                    bloco[i][j].posic = (Vector2){posx, posy};

                    if(bloco[i][j].isiton == true){
                        switch (cor){
                            case 'y' :  DrawRectangle(bloco[i][j].posic.x, bloco[i][j].posic.y, xbloco, ybloco, YELLOW);
                                        break;
                            case 'r' :  DrawRectangle(bloco[i][j].posic.x, bloco[i][j].posic.y, xbloco, ybloco, RED);
                                        break;
                            case 'b' :  DrawRectangle(bloco[i][j].posic.x, bloco[i][j].posic.y, xbloco, ybloco, BLUE);
                                        break;
                            case 'x' :  DrawRectangle(bloco[i][j].posic.x, bloco[i][j].posic.y, xbloco, ybloco, PURPLE);
                                        break;
                            case 'g' :  DrawRectangle(bloco[i][j].posic.x, bloco[i][j].posic.y, xbloco, ybloco, GREEN);
                                        break;
                        }
                    }
                }
        EndDrawing();

    return;
}

/*Cada frame do jogo passa por toda esta funcao, que eh apenas de back-end, ou seja, nada daqui eh mostrado na tela.
Funcao serve apenas para alterar posicoes de acordo com comandos do teclado*/
void fisica(){

        //Movimentacao de jogador
        if (IsKeyDown(KEY_LEFT))
            player.posic.x -= 5;
        if ((player.posic.x - player.tam.x/2) <= 0)
            player.posic.x = player.tam.x/2;
        if (IsKeyDown(KEY_RIGHT))
            player.posic.x += 5;
        if ((player.posic.x + player.tam.x/2) >= screenW)
            player.posic.x = screenW - player.tam.x/2;

        //Inicio da bola com barra de espaco
        if (!bola.isiton){
            if (IsKeyPressed(KEY_SPACE)){
                bola.isiton = true;
                bola.vel = (Vector2){0, -5};
            }
        }

        //Fisica da bola e testes de colisao

        if (bola.isiton){
            bola.posic.x += bola.vel.x;
            bola.posic.y += bola.vel.y;
        }else
            bola.posic = (Vector2){player.posic.x, screenH*7/8 - 30};

        //Colisao bola-parede
        if (((bola.posic.x + bola.raio) >= screenW) || ((bola.posic.x - bola.raio) <= 0))
            bola.vel.x *= -1;
        if ((bola.posic.y - bola.raio) <= 0)
            bola.vel.y *= -1;
        if ((bola.posic.y + bola.raio) >= screenH){
            bola.vel = (Vector2){0, 0};
            bola.isiton = false;
            player.vida--;
        }

        //Colisao bola-jogador
        if (CheckCollisionCircleRec(bola.posic, bola.raio,
            (Rectangle){player.posic.x - player.tam.x/2, player.posic.y - player.tam.y/2, player.tam.x, player.tam.y})){
            if (bola.vel.y > 0){
                bola.vel.y *= -1;
                bola.vel.x = (bola.posic.x - player.posic.x)/(player.tam.x/2)*5;
            }
        }

        //Colisao bola-bloco
        for (int i = 0; i < LINHA; i++)
            for (int j = 0; j < COLUNA; j++){
                if (bloco[i][j].isiton){
                    //Colisao embaixo
                    if (((bola.posic.y - bola.raio) <= (bloco[i][j].posic.y + ybloco/2)) &&
                        ((bola.posic.y - bola.raio) > (bloco[i][j].posic.y + ybloco/2 + bola.vel.y)) &&
                        ((fabs(bola.posic.x - bloco[i][j].posic.x)) < (xbloco/2 + bola.raio*2/3)) && (bola.vel.y < 0)){
                        bloco[i][j].isiton = false;
                        bola.vel.y *= -1;
                        player.pontos += 10;
                    }
                    //Colisao em cima
                    else if (((bola.posic.y + bola.raio) >= (bloco[i][j].posic.y - ybloco/2)) &&
                            ((bola.posic.y + bola.raio) < (bloco[i][j].posic.y - ybloco/2 + bola.vel.y)) &&
                            ((fabs(bola.posic.x - bloco[i][j].posic.x)) < (xbloco/2 + bola.raio*2/3)) && (bola.vel.y > 0)){
                        bloco[i][j].isiton = false;
                        bola.vel.y *= -1;
                        player.pontos += 10;
                    }
                    //Colisao lateral esquerda
                    else if (((bola.posic.x + bola.raio) >= (bloco[i][j].posic.x - xbloco/2)) &&
                            ((bola.posic.x + bola.raio) < (bloco[i][j].posic.x - xbloco/2 + bola.vel.x)) &&
                            ((fabs(bola.posic.y - bloco[i][j].posic.y)) < (ybloco/2 + bola.raio*2/3)) && (bola.vel.x > 0))
                    {
                        bloco[i][j].isiton = false;
                        bola.vel.x *= -1;
                        player.pontos += 10;
                    }
                    //Colisao lateral direita
                    else if (((bola.posic.x - bola.raio) <= (bloco[i][j].posic.x + xbloco/2)) &&
                            ((bola.posic.x - bola.raio) > (bloco[i][j].posic.x + xbloco/2 + bola.vel.x)) &&
                            ((fabs(bola.posic.y - bloco[i][j].posic.y)) < (ybloco/2 + bola.raio*2/3)) && (bola.vel.x < 0))
                    {
                        bloco[i][j].isiton == false;
                        bola.vel.x *= -1;
                        player.pontos += 10;
                    }
                }
            }

        //Funcionamento do game over
        if (player.vida <= 0){
            gameOver = true;
        }else{
            gameOver = true;//Isso eh uma forma artificial de testar se o jogo terminou. Caso fosse fazer um if dentro do for, ia ser muito mais chato. Veja no proximo comentario.
            for (int i = 0; i < LINHA; i++)
                for (int j = 0; j < COLUNA; j++){
                    if (bloco[i][j].isiton == true)
                        gameOver = false;//Definiu-se game over como verdadeiro acima para que, se ao menos UM bloco estiver ligado, torne a ser falso
                }
        }
}

//Funcao que organiza todas as funcoes relativas ao funcionamento do jogo. Eh a funcao mais externa, a qual eh aberta pela opcao "INICIAR JOGO" do menu.
void roda_jogo(){
    int fase = 1;//Inicializa a fase como 1
    char nivel[5][10] = {0};//Por organizacao, anula todos os espacos da matriz

    do{//Faz pelo menos uma vez o loop
        start(nivel, fase);//Inicializa o jogo passando como referencia a matriz nivel e a fase atual

        while(!WindowShouldClose() && opt != MENU){

            fisica();//Funcao de controle do teclado
            traduz_mapa(nivel);//Traduz o frame a ser mostrado na tela a partir das variaveis globais de posicao e as cores atraves do nivel

            if(IsKeyPressed(KEY_TAB) || player.vida <= 0)//Mecanismo de retorno ao menu para o usuario poder utilizar o tab
                opt = MENU;

            if(gameOver == true && player.vida > 0){//Se o jogo acabar mas a vida ainda for maior que zero eh porque o jogador passou de fase
                gameOver = false;//Desmente o game over
                fase++;//Aumenta a fase
                break;//Sai do laco While mais interno
            }
        }
    }while(!WindowShouldClose() && opt != MENU && gameOver == false);

    if(opt != MENU){//Se sair do laco while sem ser por opt = MENU, ele fecha a janela
        CloseWindow();
    }else//Caso contrario, ele volta ao menu (pois opt = MENU) e retorna o jogo para o estado zero (descarrega o jogo).
        start(nivel, MENU);/*Funcao start utilizada de maneira nao trivial, aproveitando seu algoritmo para descarregar
        o jogo passando "fase" como MENU*/

    return;
}

//Funcao que meramente imprime o scoreboard na tela.
void high_scores(){
    abre_scores(LEITURA);//Chama a funcao abre_scores no modo de leitura

    //Funcao praticamente front-end-only, atua apenas imprimindo dados do vetor de structs de Scores
    while(!WindowShouldClose() && opt != MENU){
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText(TextFormat("%s", scores[0].nome), 0, 0, 50, BLACK);
            DrawText(TextFormat("%d", scores[0].pontos), 400, 0, 50, BLACK);
            DrawText(TextFormat("%s", scores[1].nome), 0, 90, 50, BLACK);
            DrawText(TextFormat("%d", scores[1].pontos), 400, 90, 50, BLACK);
            DrawText(TextFormat("%s", scores[2].nome), 0, 180, 50, BLACK);
            DrawText(TextFormat("%d", scores[2].pontos), 400, 180, 50, BLACK);
            DrawText(TextFormat("%s", scores[3].nome), 0, 270, 50, BLACK);
            DrawText(TextFormat("%d", scores[3].pontos), 400, 270, 50, BLACK);
            DrawText(TextFormat("%s", scores[4].nome), 0, 360, 50, BLACK);
            DrawText(TextFormat("%d", scores[4].pontos), 400, 360, 50, BLACK);
        EndDrawing();

        if(IsKeyPressed(KEY_TAB))//Mecanismo de retorno ao menu
            opt = MENU;
    }
    if(opt != MENU)//Caso saia do laco while sem retorno ao menu, fecha a janela
        CloseWindow();
}

/*Unica funcao inteira do programa, eh aplicada em um switch-case na main e retorna 1, 2 ou 3 dependendo
de quanto valia o option quando o enter foi clicado.*/
int menu(void){
    //Define a posicao do centro da primeira, segunda e terceira opcao do menu
    static const float pos1 = 237.5;
    static const float pos2 = 267.5;
    static const float pos3 = 297.5;

    //Define a posicao inicial da selecao do menu
    Vector2 option = {(float)screenW/2, pos1};

    Texture2D logo = LoadTexture("logo.png");//Abre a logo

    while(!WindowShouldClose()){
        //Comandos de seta
        if(IsKeyPressed(KEY_UP)){
            option.y -= select;
            if(option.y < pos1) option.y = pos3;
        }
        if(IsKeyPressed(KEY_DOWN)){
            option.y += select;
            if(option.y > pos3) option.y = pos1;
        }
        //Um frame
        BeginDrawing();
            ClearBackground(BLACK);

            DrawTexture(logo, GetScreenWidth()/2-(float)logo.width/2, 20, WHITE);

            //Desenha selecao na tela
            if(option.y == pos1)
                DrawRectangle(GetScreenWidth()/2-MeasureText("INICIAR JOGO", 20)/2,
                pos1, MeasureText("INICIAR JOGO", 20), 20, GREEN);

            else if(option.y == pos2)
                DrawRectangle(GetScreenWidth()/2-MeasureText("HIGH SCORES", 20)/2,
                pos2, MeasureText("HIGH SCORES", 20), 20, BLUE);

            else if(option.y == pos3)
                DrawRectangle(GetScreenWidth()/2-MeasureText("SAIR", 20)/2,
                pos3, MeasureText("SAIR", 20), 20, RED);

            //Desenha opcoes na tela
            DrawText("INICIAR JOGO", GetScreenWidth()/2-MeasureText("INICIAR JOGO", 20)/2, pos1, 20, WHITE);

            DrawText("HIGH SCORES", GetScreenWidth()/2-MeasureText("HIGH SCORES", 20)/2, pos2, 20, WHITE);

            DrawText("SAIR", GetScreenWidth()/2-MeasureText("SAIR", 20)/2, pos3, 20, WHITE);

        EndDrawing();

        //Encerra a funcao com um valor entre 1 e 3 dependendo de onde a selecao estava quando o enter foi clicado
        if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)){
            if(option.y == pos1)
                return 1;

            else if(option.y == pos2)
                return 2;

            else if(option.y == pos3)
                return 3;
        }
    }

    CloseWindow();

    return 0;
}


int main(void)
{
    InitWindow(screenW, screenH, "Breakout Lucas e Vitor"); //Abre a janela

    SetTargetFPS(60); //Quantidade MAXIMA de FPS/quadros por segundo (depende de hardware). Cada vez que a funcao DrawGame eh chamada, ela inicia um laco que se repete 60 vezes por segundo.

    //Loop mais externo do programa, que lanca um switch-case a depender de uma variavel global controlada pela funcao inteira "menu".
    while(!WindowShouldClose()){
        do{
            opt = menu();

            switch (opt){
            case 1 :     roda_jogo();
                         break;
            case 2 :     high_scores();
                         break;
            case 3 :     CloseWindow();
                         break;
            }
        }while(opt == MENU);//Repete infitamente enquanto a variavel global opt for igual a MENU.
    }

    CloseWindow();

    return 0;
}

