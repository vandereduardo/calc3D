# Plano de desenvolvimento

## Fase 0 — Preparar o projeto

- [x] Criar reconhecimento funcional e matriz inicial de funcionalidades.
- [x] Registrar os requisitos de G-code, cálculo, tarifa elétrica e orçamento.
- [x] Escolher licença MIT e documentar o modelo inicial de contribuição.
- [ ] Escolher tecnologia desktop e definir versão mínima do Windows.
- [ ] Definir convenção para issues, branches, commits e revisão.
- [ ] Identificar fontes confiáveis para tarifas por UF e bandeiras.
- [x] Inspecionar uma amostra local da Kobra X exportada pelo Anycubic Slicer Next 2.0.0.3 e registrar campos observados.
- [ ] Definir e validar mais amostras próprias de G-code, incluindo um trabalho multicolor.

## Fase 1 — Especificar o MVP

- [ ] Fechar quais fatiadores e metadados de G-code entram na primeira versão; começar pelo perfil Kobra X do Anycubic Slicer Next.
- [ ] Definir comportamento para arquivo inválido, metadados incompletos, múltiplos materiais/extrusores e edição manual após importação.
- [ ] Especificar fórmulas, unidades, arredondamento, intervalos e mensagens de validação.
- [ ] Definir perfis de impressora e filamento, armazenamento/histórico e backup/exportação.
- [ ] Definir campos do orçamento PDF e como o logotipo é armazenado.
- [ ] Criar wireframes próprios para importação, cálculo, resultado e orçamento.

## Fase 2 — Implementar o núcleo

- [ ] Implementar modelo de cálculo independente da interface.
- [ ] Implementar validação e testes de unidade para cada componente e fórmula.
- [ ] Implementar parser G-code por adaptadores de fatiador, com testes sintéticos e amostras próprias por combinação.
- [ ] Implementar catálogo editável de materiais, impressoras e tarifas.
- [ ] Construir a interface Windows e os fluxos manuais e de importação.
- [ ] Criar orçamento PDF a partir de dados locais.

## Fase 3 — Preparar distribuição e comunidade

- [ ] Criar CI para testes, lint/formatação e empacotamento.
- [ ] Revisar tratamento de arquivos, dependências e atualizações.
- [ ] Escrever guia de instalação, FAQ, changelog e notas de versão.
- [ ] Criar releases assinadas e instruções de verificação.
- [ ] Definir processo para correções de segurança e atualização de tarifas.
- [ ] Validar instalação e funcionamento em máquina Windows limpa.

## Fase 4 — Suporte macOS

- [ ] Confirmar que a arquitetura e as bibliotecas escolhidas suportam macOS.
- [ ] Definir versões mínimas e requisitos de assinatura/notarização.
- [ ] Testar instalador, permissões de arquivos, impressão e exportação PDF.

## Critérios para chamar a primeira versão de MVP

- Importação local de cada formato G-code declarado como suportado, com estado claro para dados encontrados e ausentes.
- Fluxo manual completo mesmo sem arquivo G-code.
- Resultados reproduzíveis, unidades claras e custos discriminados.
- Tarifas com fonte e vigência visíveis, sempre substituíveis pelo usuário.
- PDF gerado localmente, sem envio de G-code ou dados do orçamento.
- Testes de cálculo e parser aprovados; pacote instalado e verificado no Windows suportado.
- README, licença, contribuição, privacidade e limitações publicados.
