# Decisões pendentes

Estas decisões devem ser fechadas antes do desenvolvimento correspondente. Não são limitações técnicas inevitáveis; são escolhas do produto e do mantenedor.

## Licença

**Estado: decidida — MIT.** O arquivo [LICENSE](../LICENSE) contém o texto escolhido.

MIT foi escolhida para facilitar uso, forks e redistribuição, inclusive comercial, com preservação dos avisos de copyright/licença. O titular indicado no cabeçalho é o identificador público `vandereduardo`; confirme antes do primeiro release se prefere usar outro nome.

## Tecnologia desktop

**Estado: pendente.** Avaliar opções com suporte maduro a Windows e macOS, atualização automática, leitura local de arquivos, impressão/PDF, acessibilidade, assinatura de pacotes e distribuição. A escolha deve considerar a experiência de contribuição de pessoas que usarão Windows.

## G-code

**Estado: primeira combinação observada; implementação e validação adicional pendentes.** Uma amostra local da Anycubic Kobra X exportada pelo Anycubic Slicer Next 2.0.0.3 foi inspecionada e contém metadados de impressora, tempo final, consumo total e por material, tipo/cor dos filamentos e dados de purga. O manual oficial também lista AC Slicer, PrusaSlicer e Cura como fatiadores candidatos para expansão após obter arquivos próprios. A fabricante descreve a Kobra X como multicolor, então considerar consumo por material e purga quando os metadados estiverem disponíveis.

“Vários formatos” será tratado como suporte progressivo a extensões e dialetos de metadados, não como promessa de que todo arquivo G-code será entendido. Implementar um núcleo que reconheça comandos G-code relevantes e adaptadores para comentários/metadados de cada fatiador. Registrar fatiador/versão detectados, campos encontrados e nível de confiança. Arquivos sem metadados suficientes continuam utilizáveis com preenchimento manual. Definir amostras próprias para validar extensões, tempo, consumo por material, purga, arquivos inválidos, compressão e edição após importação.

Detalhes dos campos e limitações estão em [Compatibilidade G-code](gcode-compatibility.md). Referências oficiais: [manual da Kobra X (PDF)](https://wiki.anycubic.com/kobra-x/M02010203-Kobra%20X%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C-EN.pdf), [página da Kobra X](https://wiki.anycubic.com/en/fdm-3d-printer/anycubic-kobra-x) e [guia do Anycubic Slicer Next](https://wiki.anycubic.com/en/software-and-app/new-page-anycubic-slicer-beta(orca-version)/anycubic-slicer-next-slicing-software-quick-start-guide).

## Tarifas de energia

**Estado: pendente.** Os valores transcritos no reconhecimento são apenas os que estavam no seletor da referência no dia da observação. Antes de oferecer valores no aplicativo, verificar fonte oficial, método de cálculo estadual, componentes incluídos (tributos e encargos), área da distribuidora, data de vigência e frequência de atualização. A tarifa manual deverá continuar disponível.

## Privacidade e armazenamento

**Estado: pendente.** Definir se histórico e perfis ficam em banco local ou arquivos, como exportar/importar e apagar esses dados, se haverá telemetria (desativada por padrão é recomendável) e como garantir que G-code e informações de orçamento não sejam transmitidos sem ação explícita.

## Distribuição e manutenção

**Estado: pendente.** Definir versões mínimas do Windows/macOS, canal de atualização, assinatura/notarização, plataforma para releases, política de suporte e contato para vulnerabilidades.
