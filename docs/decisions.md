# Decisões pendentes

Estas decisões devem ser fechadas antes do desenvolvimento correspondente. Não são limitações técnicas inevitáveis; são escolhas do produto e do mantenedor.

## Licença

**Estado: decidida — MIT.** O arquivo [LICENSE](../LICENSE) contém o texto escolhido.

MIT foi escolhida para facilitar uso, forks e redistribuição, inclusive comercial, com preservação dos avisos de copyright/licença. O titular indicado no cabeçalho é o identificador público `vandereduardo`; confirme antes do primeiro release se prefere usar outro nome.

## Tecnologia desktop

**Estado: decidida — C++ com Qt Widgets e CMake.** A aplicação terá uma única tela, sem camada web. Qt oferece caminho de interface para Windows e macOS; CMake organiza a configuração e compilação. O MVP abre G-code localmente, mostra os dados reconhecidos e os campos que faltam, permite preenchê-los e recalcula os valores na própria tela. Não haverá geração de PDF, conta, servidor ou histórico obrigatório. A tela, o núcleo de cálculo e o parser estão integrados em `src/app/`, `src/calculation/` e `src/gcode/`; a validação com amostras adicionais permanece pendente.

Para Windows, a distribuição inicial será uma pasta portátil ou instalador com o `.exe` e as bibliotecas Qt necessárias. Um único executável estático pode ser reavaliado depois; aumenta o tamanho e tem restrições com plugins. macOS receberá um pacote `.app` quando a versão Windows estiver estável. A associação de extensões G-code será configurada no empacotamento de cada plataforma.

Qt está sob licenças LGPL/GPL e comerciais conforme os módulos e a forma de distribuição. O projeto continuará MIT; incluir Qt requer cumprir os termos da licença aplicável, manter avisos e distribuir dependências conforme necessário. Revisar módulos e obrigações antes de publicar releases.

Os protótipos atuais de parser e cálculo em JavaScript são referências exploratórias; a implementação do aplicativo será C++. Preservar regras e casos já documentados durante a migração, sem depender de Node.js no produto final.

## G-code

**Estado: parser genérico e dois formatos observados; validação adicional pendente.** Há uma amostra da Anycubic Kobra X com Anycubic Slicer Next 2.0.0.3 e outra da Bambu Lab A1 com Bambu Studio 02.08.02.61. O parser C++ lê comentários em fluxo, reconhece modelos desconhecidos e importa os campos que cada arquivo fornece. A amostra Anycubic inclui gramas por material; a amostra Bambu fornece somente o peso total, então a divisão por material permanece manual. O manual da Kobra X lista AC Slicer, PrusaSlicer e Cura como candidatos; OrcaSlicer também será avaliado por usar convenções semelhantes às do Bambu Studio. Não prometer suporte universal antes de validar cada combinação de fatiador, modelo e versão.

“Vários formatos” será tratado como suporte progressivo a extensões e dialetos de metadados, não como promessa de que todo arquivo G-code será entendido. Implementar um núcleo que reconheça comandos G-code relevantes e adaptadores para comentários/metadados de cada fatiador. Registrar fatiador/versão detectados, campos encontrados e nível de confiança. Arquivos sem metadados suficientes continuam utilizáveis com preenchimento manual. Definir amostras próprias para validar extensões, tempo, consumo por material, purga, arquivos inválidos, compressão e edição após importação.

Detalhes dos campos e limitações estão em [Compatibilidade G-code](gcode-compatibility.md). Referências oficiais: [manual da Kobra X (PDF)](https://wiki.anycubic.com/kobra-x/M02010203-Kobra%20X%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C-EN.pdf), [página da Kobra X](https://wiki.anycubic.com/en/fdm-3d-printer/anycubic-kobra-x) e [guia do Anycubic Slicer Next](https://wiki.anycubic.com/en/software-and-app/new-page-anycubic-slicer-beta(orca-version)/anycubic-slicer-next-slicing-software-quick-start-guide).

## Tarifas de energia

**Estado: catálogo de referência implementado; validação oficial pendente.** O combo inclui as 27 UFs e distribuidoras exibidas pela [calculadora Onyon 3D](https://onyon3-d-calculator.vercel.app/), com os valores observados em 25/10/2026. Ao escolher uma UF, a tarifa média preenche o campo aplicado, que continua editável. A bandeira permanece separada. Esses números são referências estáticas copiadas do site, não são buscados automaticamente e ainda precisam de validação de fonte oficial, método de cálculo, componentes incluídos (tributos e encargos), área da distribuidora e vigência. A tarifa manual deve continuar disponível.

## Privacidade e armazenamento

**Estado: pendente.** Definir se haverá histórico e perfis locais e como exportar/importar e apagar esses dados. O MVP deve calcular localmente, sem telemetria e sem transmitir G-code ou valores inseridos.

## Distribuição e manutenção

**Estado: pendente.** Definir versões mínimas do Windows/macOS, canal de atualização, assinatura/notarização, plataforma para releases, política de suporte e contato para vulnerabilidades.
