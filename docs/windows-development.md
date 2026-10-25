# Desenvolvimento no Windows

Este guia prepara a máquina Windows para abrir e continuar o calc3D. O projeto usa C++17, Qt 6 Widgets e CMake. O código atual é apenas um esqueleto da interface: ainda não lê G-code nem calcula custos.

## Ferramentas necessárias

1. **Git for Windows**, para clonar o repositório quando estiver disponível nessa máquina, ou extrair a cópia local do projeto.
2. **Visual Studio 2022** ou **Build Tools for Visual Studio 2022**, com a carga de trabalho **Desenvolvimento para desktop com C++** e um Windows SDK.
3. **Qt 6 para MSVC 2022 64-bit**, incluindo o módulo **Qt Widgets**. Use a mesma versão principal/secundária de Qt durante desenvolvimento e empacotamento.
4. **CMake 3.21 ou superior**. O instalador do Visual Studio pode fornecer CMake; também pode ser instalado separadamente.

O compilador Qt, a arquitetura e a versão do compilador precisam coincidir. Este projeto foi preparado para `MSVC 2022 64-bit`; não misture bibliotecas Qt MinGW com o compilador MSVC.

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
- Os campos exibidos ainda não estão conectados ao parser ou às fórmulas.
- Os protótipos JavaScript permanecem como referência; o produto será portado para C++.
- Ainda não há testes automatizados, instalador nem pacote de release.
