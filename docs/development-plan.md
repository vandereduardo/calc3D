# Plano de desenvolvimento

## Fase 0 — Preparar o projeto

- [x] Criar reconhecimento funcional e matriz inicial de funcionalidades.
- [x] Registrar os requisitos de G-code, cálculo, tarifa elétrica e orçamento.
- [x] Escolher licença MIT e documentar o modelo inicial de contribuição.
- [x] Escolher C++ com Qt Widgets e CMake; definir a versão mínima do Windows antes de empacotar.
- [ ] Definir convenção para issues, branches, commits e revisão.
- [ ] Identificar fontes confiáveis para tarifas por UF e bandeiras.
- [x] Inspecionar uma amostra local da Kobra X exportada pelo Anycubic Slicer Next 2.0.0.3 e registrar campos observados.
- [ ] Definir e validar mais amostras próprias de G-code, incluindo um trabalho multicolor.

## Fase 1 — Especificar o MVP

- [ ] Fechar quais fatiadores e metadados de G-code entram na primeira versão; começar pelo perfil Kobra X do Anycubic Slicer Next.
- [ ] Definir comportamento para arquivo inválido, metadados incompletos, múltiplos materiais/extrusores e edição manual após importação.
- [ ] Especificar fórmulas, unidades, arredondamento, intervalos e mensagens de validação.
- [ ] Definir perfis de impressora e filamento, armazenamento/histórico e backup/exportação.
- [x] Definir o fluxo inicial como uma tela única de importação, campos editáveis e resultados; PDF fora do escopo.
- [ ] Especificar o wireframe da tela única, incluindo dados ausentes, erros e resultado atualizado.

## Fase 2 — Implementar o núcleo

- [x] Criar protótipo do modelo de cálculo independente da interface; validação ainda pendente.
- [ ] Revisar e validar o protótipo local `src/calculation/calculate-cost.mjs` com exemplos conhecidos antes de integrar à interface.
- [ ] Implementar validação e testes de unidade para cada componente e fórmula.
- [ ] Revisar e validar o protótipo local `src/gcode/parser.mjs` com amostras sintéticas e próprias antes de declarar suporte.
- [ ] Implementar adaptadores adicionais de fatiador após validação do primeiro parser.
- [ ] Implementar catálogo editável de materiais, impressoras e tarifas.
- [ ] Migrar os protótipos de cálculo e G-code de JavaScript para C++ e ligar os campos da tela ao cálculo.
- [x] Criar esqueleto local da tela Qt Widgets e da configuração CMake; ainda sem parser/cálculo integrados.
- [x] Remover geração de PDF do escopo inicial; apresentar o detalhamento e o preço calculado na própria tela.

## Fase 3 — Preparar distribuição e comunidade

- [ ] Criar CI para testes, lint/formatação e empacotamento.
- [ ] Revisar tratamento de arquivos, dependências e atualizações.
- [x] Escrever guia inicial de preparação e compilação no Windows; FAQ, changelog e notas de versão ficam para releases.
- [ ] Criar releases assinadas e instruções de verificação.
- [ ] Definir processo para correções de segurança e atualização de tarifas.
- [ ] Validar instalação e funcionamento em máquina Windows limpa.

## Fase 4 — Suporte macOS

- [x] Confirmar que Qt oferece suporte a Windows e macOS; validar a distribuição macOS na fase própria.
- [ ] Definir versões mínimas e requisitos de assinatura/notarização.
- [ ] Testar instalador, permissões de arquivos e associação de G-code ao aplicativo.

## Critérios para chamar a primeira versão de MVP

- Importação local de cada formato G-code declarado como suportado, com estado claro para dados encontrados e ausentes.
- Fluxo manual completo mesmo sem arquivo G-code.
- Resultados reproduzíveis, unidades claras e custos discriminados.
- Tarifas com fonte e vigência visíveis, sempre substituíveis pelo usuário.
- Tela única apresenta componentes do custo e preço calculado; campos ausentes podem ser preenchidos manualmente.
- Testes de cálculo e parser aprovados; pacote instalado e verificado no Windows suportado.
- README, licença, contribuição, privacidade e limitações publicados.
