#!/bin/bash

# -----------------------------
# Ajuda
# -----------------------------
if [ "${1:-}" = "help" ]; then
    echo "Uso: ./run.sh [num_tasks] [N]"
    echo
    echo "Descricao:"
    echo "  Script principal do projeto."
    echo "  Compila scheduler/ e prepare/, limpa dados antigos,"
    echo "  gera tarefas e abre um menu interativo para"
    echo "  comparar modos de otimização sobre as mesmas tarefas."
    echo
    echo "Argumentos:"
    echo "  num_tasks  - numero de tarefas a gerar/executar (default: 10)"
    echo "  N          - maximo de processos em paralelo (default: 4)"
    echo
    echo "Exemplos:"
    echo "  ./run.sh          # defaults (10 tarefas, N=4)"
    echo "  ./run.sh 20       # 20 tarefas, N=4"
    echo "  ./run.sh 20 8     # 20 tarefas, N=8"
    echo
    exit 0
fi

# -----------------------------
# 0) Ler e validar argumentos
# -----------------------------
NUM_TASKS="${1:-10}" #guarda na variável NUM_TASKS o 1 argumento. Se o primeiro argumento não for fornecido, usa 10 como padrão
N="${2:-4}"        #guarda na variável N o 2 argumento. Se o segundo argumento não for fornecido, usa 4 como padrão

if [ "$NUM_TASKS" -le 0 ]; then #-le significa "less than or equal" (menor ou igual)
    echo "Erro: num_tasks tem de ser > 0"
    echo "Uso: ./run.sh [num_tasks] [N]  (ou ./run.sh help)"
    exit 1
fi

if [ "$N" -le 0 ]; then
    echo "Erro: N tem de ser >= 1"
    echo "Uso: ./run.sh [num_tasks] [N]  (ou ./run.sh help)"
    exit 1
fi

# -----------------------------
# 1) Configuração do projeto
# -----------------------------
SCHED_DIR="scheduler" #diretório do scheduler para depois conseguirmos compilar e executar
PREP_DIR="prepare"   #diretório do prepare para depois conseguirmos compilar e executar
TASKS_DIR="tasks_data" #diretório onde guardamosas tarefas geradas

SCHED_BIN="$SCHED_DIR/scheduler"  #caminho do binário/executável do scheduler
PREP_BIN="$PREP_DIR/prepare"     #caminho do binário/executável do prepare

#Intervalo de durações geradas aleatoriamente (em segundos)
MIN_DUR=1
MAX_DUR=5

echo "----------------------------------------"
echo "CONFIG:"
echo "  num_tasks = $NUM_TASKS" #assumimos que o utilizador quer gerar num_tasks tarefas, ou seja, são criadas tantas tasks quanto são executadas
echo "  N         = $N  (usado apenas nos modos paralelos 1 e 3)"
echo "  duracao   = [$MIN_DUR..$MAX_DUR] (random)"
echo "----------------------------------------"
echo

# -----------------------------
# 2) Compilação (usar Makefile das pastas)
# -----------------------------
if [ ! -d "$SCHED_DIR" ] || [ ! -f "$SCHED_DIR/Makefile" ]; then #verifica se o diretório do scheduler existe e se o Makefile existe
    #-d verifica se o diretório existe, -f verifica se o arquivo existe
    echo "Erro: nao encontrei $SCHED_DIR/Makefile. Não foi possível compilar o programa."
    exit 1
fi

if [ ! -d "$PREP_DIR" ] || [ ! -f "$PREP_DIR/Makefile" ]; then
    echo "Erro: nao encontrei $PREP_DIR/Makefile"
    exit 1
fi

echo "--- A compilar ---"
make -C "$SCHED_DIR" clean-all #entra na pasta scheduler e corre o make lá dentro. Limpa object files, o executável e as estatísticas antigas
make -C "$SCHED_DIR"           #compila o scheduler
make -C "$PREP_DIR" clean-all  #entra na pasta prepare e corre o make lá dnetro. Limpa object files e todos os binários gerados
make -C "$PREP_DIR"            #compila o prepare
echo "Compilacao concluida."
echo

# Garantir que os binários existem
#-x "file" verifica se o arquivo existe e é executável
[ -x "$SCHED_BIN" ] || { echo "Erro: executavel nao encontrado: $SCHED_BIN"; exit 1; }
[ -x "$PREP_BIN" ]  || { echo "Erro: executavel nao encontrado: $PREP_BIN"; exit 1; }


# -----------------------------
# 4) Gerar tarefas (prepare)
# -----------------------------
echo "--- A gerar $NUM_TASKS tarefas ---"
# Criamos a pasta na raiz para garantir que ela existe
mkdir -p "$TASKS_DIR" 

for (( id=1; id<=NUM_TASKS; id++ )); do
    DUR=$(( ( RANDOM % (MAX_DUR - MIN_DUR + 1) ) + MIN_DUR ))
    
    # O SEGREDO ESTÁ AQUI: Entramos na pasta prepare para que o 
    # "../tasks_data" do código C aponte para a raiz do projeto.
    ( cd "$PREP_DIR" && ./prepare "$id" "$DUR" ) >/dev/null
    
    echo "  -> Task id=$id duration=${DUR}s"
done
echo

# sanity check: pelo menos 1 bin existe
[ -f "$TASKS_DIR/task_1.bin" ] || { echo "Erro: nao foram gerados .bin em $TASKS_DIR"; exit 1; }

# -----------------------------
# Função: executar scheduler num modo
# -----------------------------
executar_modo() {
    local MODO="$1"

    echo
    echo "--- A executar scheduler ---"
    case "$MODO" in
        0)
            echo "[MODO 0] Sem Otimizações (FCFS sequencial)  (ignora N)"
            ( cd "$SCHED_DIR" && ./scheduler "$NUM_TASKS" )
            ;;
        1)
            echo "[MODO 1] Otimização do Paralelismo (FCFS paralelo, N=$N)"
            ( cd "$SCHED_DIR" && ./scheduler "$NUM_TASKS" "$N" fcfs )
            ;;
        2)
            echo "[MODO 2] Otimização do SJF (SJF sequencial) (ignora N)"
            ( cd "$SCHED_DIR" && ./scheduler "$NUM_TASKS" 1 sjf )
            ;;
        3)
            echo "[MODO 3] Todas as Otimizações (SJF + paralelo, N=$N)"
            ( cd "$SCHED_DIR" && ./scheduler "$NUM_TASKS" "$N" sjf )
            ;;
        *)
            echo "Modo invalido. Use 0, 1, 2 ou 3."
            return 1
            ;;
    esac
    return 0
}

# -----------------------------
# 5) Menu interativo
# -----------------------------
echo "==============================================="
echo "   MENU DE UTILIZADOR - COMPARAÇÃO DE MODOS"
echo "==============================================="
echo

while true; do
    echo "----------------------------------------"
    echo " [MODO 0] Sem Otimizações (FCFS sequencial)"
    echo " [MODO 1] Otimização do Paralelismo (FCFS paralelo (N=$N))"
    echo " [MODO 2] Otimização do SJF (SJF sequencial)"
    echo " [MODO 3] Todas as Otimizações (SJF + paralelo (N=$N))"
    echo " [MODO 4] Ver Estatísticas e SAIR"
    echo "----------------------------------------"
    read -p "Escolhe o modo (0-4): " OPCAO 

    if [[ ! "$OPCAO" =~ ^[0-4]$ ]]; then # =~ é para regex em bash. ^[0-4]$ significa "um dígito entre 0 e 4". 
        echo "ERRO: Opção inválida! Escolhe um número entre 0 e 4."
        echo
        continue
    fi

    if [ "$OPCAO" = "4" ]; then
        echo
        echo "A terminar o programa..."
        break
    fi

    executar_modo "$OPCAO"

    echo
    echo "Modo $OPCAO concluído!"
    echo
    read -p "Pressiona ENTER para voltar ao menu..." _
    echo
done

# -----------------------------
# 6) Estatísticas finais
# -----------------------------
echo
echo "========================================"
echo "      ESTATÍSTICAS FINAIS"
echo "========================================"
echo

if [ -f "$SCHED_DIR/Estatisticas_Globais.txt" ]; then
    cat "$SCHED_DIR/Estatisticas_Globais.txt"
else
    echo "Aviso: Ficheiro 'Estatisticas_Globais.txt' não foi criado."
fi

echo
echo "Feito."
