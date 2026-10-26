# calc3D

Aplicativo desktop local para calcular e apresentar na tela o custo de impressões 3D. A primeira plataforma planejada é Windows; macOS virá depois. A pessoa poderá abrir ou selecionar um G-code, revisar os dados importados, preencher os campos que faltam e ver o resultado atualizado na mesma tela.

> **Status: protótipo local.** O núcleo de cálculo em C++ aceita valores manuais, importa tempo e consumo de metadados G-code e inclui tarifas médias de referência por UF. O parser e as tarifas ainda precisam de validação com amostras e fontes oficiais. Persistência e testes automatizados seguem pendentes.

## O que está sendo planejado

- Leitura local de G-code de vários fatiadores, começando pela Anycubic Kobra X; preenchimento de duração e consumo por material quando os metadados estiverem disponíveis.
- Cálculo de filamento, energia, desgaste da impressora, falhas, mão de obra, custos fixos, embalagem, frete, taxas e lucro.
- Tarifas elétricas estimadas por UF, bandeiras tarifárias, data de referência e edição manual.
- Discriminação dos custos e atualização imediata do resultado na tela.
- Aplicativo desktop C++/Qt para Windows, com caminho de suporte a macOS.

## Documentação

- [Preparar o ambiente e compilar no Windows](docs/windows-development.md)
- [Reconhecimento funcional e requisitos](replica/recon.md)
- [Matriz de funcionalidades](replica/features.csv)
- [Compatibilidade G-code](docs/gcode-compatibility.md)
- [Núcleo de cálculo](docs/calculation-core.md)
- [Plano de desenvolvimento](docs/development-plan.md)
- [Decisões pendentes](docs/decisions.md)
- [Como contribuir](CONTRIBUTING.md)

## Princípios do projeto

- Cálculos e processamento de G-code devem funcionar localmente; os arquivos não serão enviados para um servidor por padrão.
- Tarifas elétricas precisam indicar fonte, abrangência, data de vigência e limites da estimativa. O valor da conta de energia do usuário sempre poderá substituir a média.
- A interface, o nome, os textos e os elementos visuais serão próprios. A referência funcional está documentada para compreender os requisitos, sem reutilizar código, marca ou ativos de terceiros.
- Fórmulas e resultados devem ser transparentes e verificáveis.

## Licença

O projeto usa a licença [MIT](LICENSE). Ela permite uso, modificação e redistribuição, inclusive comercial, desde que os avisos de copyright e licença sejam preservados. Leia o texto completo da licença antes de reutilizar o código.

## Estado atual

O projeto tem requisitos documentados e um aplicativo Qt Widgets/CMake para Windows. A tela permite informar vários materiais, tempo, energia, desgaste e outros custos, com atualização imediata do detalhamento e do preço sugerido. O parser C++ lê metadados G-code em fluxo e preenche duração, gramas por material e identificadores disponíveis; confira os avisos e revise os valores importados antes de usar. A compatibilidade ainda não foi validada com uma coleção de amostras. Veja o [plano de desenvolvimento](docs/development-plan.md).

Com as dependências instaladas, compile e execute pelo PowerShell na raiz do projeto:

```powershell
.\scripts\build.ps1
.\scripts\run.ps1
```

Para iniciar o aplicativo com duplo clique no Explorador de Arquivos, abra `run.cmd` na raiz do repositório. Para depurar, continue usando `.\scripts\run.ps1 -Configuration Debug` no PowerShell.

O [guia Windows](docs/windows-development.md) explica a instalação das ferramentas, a seleção do Qt e a compilação em Debug.
