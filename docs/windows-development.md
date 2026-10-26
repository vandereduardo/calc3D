# Desenvolvimento no Windows

Este guia prepara a máquina Windows para abrir e continuar o calc3D. O projeto usa C++17, Qt 6 Widgets e CMake. O aplicativo calcula custos por peça e importa metadados reconhecidos em arquivos G-code; a compatibilidade do parser ainda precisa ser validada com mais amostras.

## Ferramentas necessárias

1. **Git for Windows**, para clonar o repositório quando estiver disponível nessa máquina, ou extrair a cópia local do projeto.
2. **Visual Studio 2022** ou **Build Tools for Visual Studio 2022**, com a carga de trabalho **Desenvolvimento para desktop com C++** e um Windows SDK.
3. **Qt 6 para MSVC 2022 64-bit**, incluindo o módulo **Qt Widgets**. Use a mesma versão principal/secundária de Qt durante desenvolvimento e empacotamento.
4. **CMake 3.21 ou superior**. O instalador do Visual Studio pode fornecer CMake; também pode ser instalado separadamente.

O compilador Qt, a arquitetura e a versão do compilador precisam coincidir. Este projeto foi preparado para `MSVC 2022 64-bit`; não misture bibliotecas Qt MinGW com o compilador MSVC.

## Compilar e executar pelo PowerShell

Os scripts do projeto localizam o Build Tools/Visual Studio 2022 e seu CMake por meio do `vswhere`. O Qt é procurado em `QT_ROOT` ou no kit `msvc2022_64` da versão mais recente instalada em `C:\Qt`.

Na raiz do repositório:

```powershell
.\scripts\build.ps1
.\scripts\run.ps1
```

O primeiro comando configura o CMake, compila em Release e executa `windeployqt`, deixando as dependências ao lado de `build/windows-msvc/Release/calc3D.exe`. Não é necessário alterar o PATH global. Essa pasta serve para desenvolvimento; uma distribuição ainda exige validação em uma máquina limpa.

Para escolher o kit ou compilar com símbolos de depuração:

```powershell
.\scripts\build.ps1 -Configuration Debug -QtRoot 'C:\Qt\6.10.3\msvc2022_64'
.\scripts\run.ps1 -Configuration Debug
```

Se o PowerShell bloquear scripts pela política de execução, é possível liberar somente a chamada desejada, sem alterar a política permanente:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\run.ps1
```

## Instalar as dependências por linha de comando

O Build Tools pode solicitar elevação pelo Windows. O comando abaixo instala a carga de C++, incluindo os componentes recomendados, sem reiniciar automaticamente:

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools --exact --source winget --accept-source-agreements --accept-package-agreements --override "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

Para baixar Qt sem instalar o IDE Qt Creator, use Python 3 com `aqtinstall` em um ambiente isolado. Os arquivos do Qt vêm dos servidores de distribuição do Qt. A seleção abaixo inclui Widgets e as ferramentas de implantação:

```powershell
python -m venv .tools\setup
.\.tools\setup\Scripts\python.exe -m pip install aqtinstall==3.3.0
.\.tools\setup\Scripts\python.exe -m aqt install-qt windows desktop 6.10.3 win64_msvc2022_64 --outputdir C:\Qt --archives qtbase qttools qtsvg qttranslations qtimageformats
```

O ambiente `.tools/` e o log do instalador são ignorados pelo Git. Python é usado apenas nessa instalação; não será uma dependência do aplicativo.

Referências: [instalação do Visual Studio por linha de comando](https://learn.microsoft.com/en-us/visualstudio/install/use-command-line-parameters-to-install-visual-studio) e [documentação do aqtinstall](https://aqtinstall.readthedocs.io/en/v3.3.0/getting_started.html).

## Ambiente verificado em 25/10/2026

- Visual Studio Build Tools 2022 17.14.42, com MSVC 19.44.35229 para x64.
- Windows SDK 10.0.26100.0.
- CMake 3.31.6-msvc6, fornecido pelo Build Tools.
- Qt 6.10.3 MSVC 2022 x64 em `C:\Qt\6.10.3\msvc2022_64`.
- Compilação Release e Debug concluída; ambos os executáveis iniciaram e permaneceram ativos sem erros registrados na inicialização. Isso não valida os fluxos da interface ou as regras de cálculo.

O CMake pode avisar que não encontrou cabeçalhos Vulkan, e `windeployqt` pode avisar sobre `dxcompiler.dll`/`dxil.dll`. O esqueleto atual usa Qt Widgets sem renderização Vulkan ou shaders DirectX; esses avisos não impediram a compilação ou a inicialização. Reavaliar essas dependências se o aplicativo passar a usar tais recursos gráficos.

## Abrir o projeto

No Visual Studio, use **Open a local folder** e selecione a pasta que contém `CMakeLists.txt`. Também é possível compilar em um terminal do **Developer Command Prompt for VS 2022**.

Se o Qt não estiver no caminho padrão, informe a pasta da instalação no primeiro comando. Substitua `6.x.x` pela versão instalada:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"
cmake --build build --config Release
```

Com o gerador padrão do Visual Studio, o executável de desenvolvimento fica em:

```text
build/Release/calc3D.exe
```

Para iniciar pelo terminal:

```powershell
./build/Release/calc3D.exe
```

## Preparar uma pasta portátil

Depois de compilar em Release, execute `windeployqt` da mesma instalação do Qt para copiar as bibliotecas e plugins Qt necessários ao lado do executável. Use um **Qt 6.x MSVC 2022 64-bit Command Prompt** ou informe o caminho completo da ferramenta:

```powershell
C:/Qt/6.x.x/msvc2022_64/bin/windeployqt.exe build/Release/calc3D.exe
```

Isso prepara uma pasta de aplicativo, não um único `.exe`. A distribuição final ainda deve verificar o Visual C++ Redistributable, a instalação em uma máquina Windows limpa, a associação de arquivos `.gcode` e o empacotamento/assinatura. A associação direta para abrir G-code ainda não está implementada.

## Estado conhecido

- A tela abre um seletor de arquivos e mostra o caminho escolhido.
- A interface já calcula os custos a partir dos campos manuais usando o núcleo C++.
- A importação de G-code reconhece tempo e dados de filamento disponíveis nos metadados; revise os valores importados e os avisos antes do cálculo.
- O parser C++ está em `src/gcode/`; o protótipo JavaScript permanece apenas como referência.
- As fórmulas C++ precisam de testes automatizados antes da validação do núcleo financeiro.
- Ainda não há testes automatizados, instalador nem pacote de release.
