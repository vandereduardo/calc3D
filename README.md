# calc3D

Aplicativo local para estimar custos e preparar orçamentos de impressão 3D. A primeira plataforma planejada é Windows; macOS virá depois. A ideia central é importar G-code de vários fatiadores, começar pela Anycubic Kobra X, completar os custos de produção e gerar um orçamento em PDF.

> **Status: planejamento.** Este repositório ainda não contém uma aplicação executável. As especificações funcionais estão sendo definidas antes da implementação.

## O que está sendo planejado

- Leitura local de G-code de vários fatiadores, começando pela Anycubic Kobra X; preenchimento de duração e consumo por material quando os metadados estiverem disponíveis.
- Cálculo de filamento, energia, desgaste da impressora, falhas, mão de obra, custos fixos, embalagem, frete, taxas e lucro.
- Tarifas elétricas estimadas por UF, bandeiras tarifárias, data de referência e edição manual.
- Discriminação dos custos e criação de orçamento PDF.
- Aplicativo desktop para Windows, com caminho de suporte a macOS.

## Documentação

- [Reconhecimento funcional e requisitos](replica/recon.md)
- [Matriz de funcionalidades](replica/features.csv)
- [Compatibilidade G-code](docs/gcode-compatibility.md)
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

O primeiro artefato é o levantamento de requisitos. A primeira impressora alvo é a Anycubic Kobra X; o suporte a outros modelos e fatiadores será ampliado com amostras verificadas. Ainda faltam escolher a tecnologia desktop, as fontes tarifárias e a política de armazenamento local. Veja o [plano de desenvolvimento](docs/development-plan.md).
