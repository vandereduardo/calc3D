# calc3D

Aplicativo desktop local para calcular e apresentar na tela o custo de impressões 3D. A primeira plataforma planejada é Windows; macOS virá depois. A pessoa poderá abrir ou selecionar um G-code, revisar os dados importados, preencher os campos que faltam e ver o resultado atualizado na mesma tela.

> **Status: protótipos locais.** A arquitetura escolhida é C++ com Qt Widgets e CMake. Os módulos em `src/` são protótipos de referência em JavaScript; a interface e os módulos C++ ainda estão no início e precisam de revisão e validação.

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

O projeto tem requisitos documentados e um esqueleto Qt Widgets/CMake ainda não compilado. Os protótipos JavaScript em `src/gcode/` e `src/calculation/` servem como referência para a migração a C++. A primeira impressora alvo é a Anycubic Kobra X; ainda faltam validar o parser, completar a tela e pesquisar fontes tarifárias. Veja o [plano de desenvolvimento](docs/development-plan.md).
