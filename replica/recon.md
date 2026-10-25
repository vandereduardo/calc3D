# Reconhecimento de produto — calculadora de custos para impressão 3D

## Objetivo deste documento

Registrar os fluxos e requisitos observáveis na calculadora de referência para orientar uma aplicação própria de orçamento de impressão 3D. Este é um levantamento funcional; não contém implementação. A referência consultada foi [Calculadora de Custo de Impressão 3D da 3D Prime](https://3dprime.com.br/calculadora-de-custo-de-impressao-3d/), em 25/10/2026.

O levantamento segue a abordagem de reconhecimento do projeto [replica-skill](https://github.com/Jakeschincariol/replica-skill): documentar funcionalidades e fluxos em estilo clean-room. A aplicação futura deverá ter nome, identidade visual, textos e implementação próprios. Este documento não recomenda copiar marca, código, imagens, textos ou APIs privadas da referência.

## Visão do produto pretendido

Aplicação local para calcular custo e preço de venda de peças impressas em 3D, com Windows como primeira plataforma e macOS como etapa posterior. O fluxo central deve aceitar G-code de diferentes fatiadores, extrair automaticamente tempo e consumo por material quando possível, combinar os dados com custos configuráveis e apresentar o custo e o preço recomendado. A primeira impressora alvo é a Anycubic Kobra X; o projeto usa licença MIT.

### Usuários e necessidades

- Maker que quer saber quanto custa uma impressão antes de vender ou produzir.
- Pequeno negócio que precisa acrescentar mão de obra, desgaste, falhas, embalagem, frete, taxas e lucro.
- Usuário que não possui ou não quer usar G-code e prefere preencher os valores manualmente.

### Plataformas e princípios de produto

- Primeira entrega: aplicativo local para Windows.
- Etapa posterior: suporte a macOS.
- Funcionamento offline para cálculos e leitura local dos arquivos; fontes tarifárias poderão exigir atualização online, mas valores já obtidos devem continuar disponíveis localmente.
- Interface em português brasileiro e valores em reais, com entradas manuais disponíveis para todas as estimativas.
- Dados de G-code e dados do orçamento devem ser tratados localmente por padrão. A referência informa que o logotipo enviado ao orçamento fica no navegador; a política da aplicação própria deve ser definida explicitamente.
- O projeto adotou MIT; a versão mínima do Windows/macOS e o mecanismo de distribuição continuam pendentes.

## Mapa da tela observada

Uma tela principal com cabeçalho/título, explicação e dois atalhos de ajuda (metodologia e tutorial), seguida por um formulário dividido em etapas. A área final apresenta o resultado atualizado ao editar os valores e um formulário separado para gerar orçamento em PDF. O site de referência também contém cabeçalho e rodapé de comércio eletrônico; esses elementos promocionais não são necessários para a aplicação local.

### Seções do formulário

1. **Importação G-code (opcional):** área de arrastar e soltar e botão para escolher arquivo. A página informa que são preenchidos tempo e gramas. Não foi feito upload de arquivo durante este levantamento; extensão aceita, variantes de comentários por fatiador, erros, limites e comportamento multicolor ainda precisam ser especificados e testados com arquivos de amostra próprios.
2. **Filamento:** material de referência, preço em R$/kg e quantidade em gramas. Materiais disponíveis na página observada: PLA Basic, PLA HT High Speed, ABS, PLA Impact, PETG, PLA Metálico e PLA Silk. Há ação para buscar preço no site da loja, que é específica da referência e não deve ser pressuposta na aplicação local.
3. **Energia e tempo:** horas, minutos, consumo médio em watts, estado brasileiro e valor em R$/kWh. A página sugere 120 W para Ender 3, 200 W para Bambu A1 e 350 W para Bambu X1C. São exemplos indicativos, não especificações universais.
4. **Desgaste da impressora (opcional):** preço pago pela impressora e vida útil estimada em horas. Sugestões visíveis: 3.000, 5.000 e 8.000 horas.
5. **Custos extras e lucro (opcionais):** mão de obra por peça, custo fixo por peça, taxa de falhas, opção para incluir embalagem e frete, modo de lucro percentual ou valor fixo, e taxas/impostos sobre a venda.
6. **Resultado:** custo, lucro e margem real, discriminação de componentes e visualização proporcional por parcela.
7. **Orçamento PDF:** nome/empresa, cliente, descrição, material, quantidade, prazo, validade, formas de pagamento, descontos e observações; há opção de logotipo. O formulário informa que descontos reduzem o lucro e recomenda conferir a margem.

## Fluxos funcionais observados

### Calcular manualmente

1. Informar preço por kg e gramas de filamento.
2. Informar duração (horas e minutos) e consumo médio em watts.
3. Escolher uma UF para preencher a tarifa média cadastrada; o campo de kWh permanece ajustável.
4. Opcionalmente configurar desgaste, mão de obra, custos fixos e taxa de falha.
5. Opcionalmente ativar embalagem/frete e informar o tratamento do frete.
6. Opcionalmente configurar lucro e taxas de venda.
7. Acionar “Calcular preço”. A página também informa que o resultado atualiza ao editar.
8. Ação de limpar remove os valores preenchidos no formulário principal.

### Importar G-code

1. Selecionar ou arrastar um arquivo de fatiador.
2. A interface promete preencher automaticamente tempo e gramas de filamento.
3. O restante do cálculo continua dependente dos dados de custo (preço do material, potência, tarifa e custos opcionais).

**Limite do reconhecimento:** os estados de sucesso, formato inválido, metadados ausentes, suporte de variantes e quais campos prevalecem quando o usuário edita após a importação não foram confirmados com arquivo real. Esses casos devem virar requisitos explícitos e testes de aceitação antes da implementação.

### Escopo inicial de compatibilidade G-code

A primeira impressora alvo é a Anycubic Kobra X. A fabricante lista Anycubic Slicer Next para o modelo; seu manual também cita AC Slicer, PrusaSlicer e Cura. Começar validando um arquivo próprio exportado pelo Anycubic Slicer Next com perfil Kobra X. Em seguida ampliar adaptadores para outros slicers usando arquivos de amostra próprios. A Kobra X é multicolor (quatro cores segundo a fabricante), então o parser e o modelo de custo precisam aceitar várias entradas de material e somar o custo de cada uma quando seus metadados estiverem disponíveis. Em qualquer caso, campos não encontrados devem continuar editáveis manualmente.

Referências do fabricante: [manual da Kobra X (PDF)](https://wiki.anycubic.com/kobra-x/M02010203-Kobra%20X%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C-EN.pdf), [página da Kobra X](https://wiki.anycubic.com/en/fdm-3d-printer/anycubic-kobra-x) e [guia do Anycubic Slicer Next](https://wiki.anycubic.com/en/software-and-app/new-page-anycubic-slicer-beta(orca-version)/anycubic-slicer-next-slicing-software-quick-start-guide).

### Gerar orçamento

1. Após obter resultado, preencher os dados de empresa/cliente e detalhes comerciais.
2. Opcionalmente enviar logotipo, validade, segundo meio de pagamento, descontos e observações.
3. Exportar o orçamento em PDF. Não foi gerado um PDF durante o levantamento.

## Regras de cálculo observadas na explicação da referência

Defina `t` como duração total em horas decimais (horas + minutos/60), `F` como percentual de falhas entre 0 e 100, `L` como lucro percentual, `Tx` como taxas percentuais da venda, `K` como tarifa base mais bandeira aplicável.

- **Filamento:** `(preço do kg ÷ 1000) × gramas utilizadas`.
- **Vários materiais:** extensão planejada para impressões multicolor: `soma((preço do kg do material_i ÷ 1000) × gramas_i)`. A regra acima da referência descreve um único preço e massa; o cálculo por material é um requisito novo para o app.
- **Energia:** `(watts × horas ÷ 1000) × tarifa em R$/kWh`.
- **Desgaste:** `(preço da impressora ÷ vida útil em horas) × horas de impressão`.
- **Reserva de falhas:** `(filamento + energia + desgaste) × F ÷ (100 − F)`.
- **Custo total:** itens anteriores + mão de obra + custos fixos + embalagem. A embalagem só é somada quando habilitada.
- **Lucro percentual:** custo × percentual de lucro. Alternativamente, o lucro pode ser um valor fixo em reais por peça.
- **Preço com taxas:** `(custo + lucro) ÷ (1 − Tx/100)`. Sem taxas, o preço é custo + lucro.
- **Frete:** pode ser repassado ao cliente no final, sem lucro nem taxas, ou somado ao custo para receber lucro/taxas conforme as opções da interface.
- **Margem real:** a página ilustra que um custo de R$ 100 com 30% de lucro gera R$ 130 antes de taxas e margem real de 23%.

As fórmulas são transcritas como regra observada, não como validação contábil. É necessário definir tratamento para entradas extremas, especialmente `F = 100%`, taxas iguais ou superiores a 100%, arredondamentos, valores negativos e campos incompletos.

## Tarifas estaduais observadas

Ao selecionar uma UF, a calculadora preenche o campo com uma tarifa residencial média. A interface declara que os dados são médias por estado baseadas na ANEEL e recomenda conferir a conta de luz, pois distribuidora e faixa de consumo alteram o valor. Os números abaixo foram lidos do seletor da página em 25/10/2026; são valores da referência e não foram verificados como tarifas oficiais atuais.

| UF | Estado | R$/kWh observado |
|---|---|---:|
| AC | Acre | 0,656 |
| AL | Alagoas | 0,742 |
| AM | Amazonas | 0,735 |
| AP | Amapá | 0,728 |
| BA | Bahia | 0,672 |
| CE | Ceará | 0,640 |
| DF | Distrito Federal | 0,720 |
| ES | Espírito Santo | 0,680 |
| GO | Goiás | 0,710 |
| MA | Maranhão | 0,690 |
| MG | Minas Gerais | 0,780 |
| MS | Mato Grosso do Sul | 0,685 |
| MT | Mato Grosso | 0,698 |
| PA | Pará | 0,745 |
| PB | Paraíba | 0,660 |
| PE | Pernambuco | 0,680 |
| PI | Piauí | 0,690 |
| PR | Paraná | 0,695 |
| RJ | Rio de Janeiro | 0,850 |
| RN | Rio Grande do Norte | 0,670 |
| RO | Rondônia | 0,660 |
| RR | Roraima | 0,650 |
| RS | Rio Grande do Sul | 0,725 |
| SC | Santa Catarina | 0,715 |
| SE | Sergipe | 0,670 |
| SP | São Paulo | 0,760 |
| TO | Tocantins | 0,675 |

### Bandeiras observadas

Valores somados à tarifa base: verde +R$ 0,00/kWh; amarela +R$ 0,01885/kWh; vermelha patamar 1 +R$ 0,04463/kWh; vermelha patamar 2 +R$ 0,07877/kWh. A interface arredonda visualmente os três últimos valores para 0,019, 0,045 e 0,079.

### Requisito para dados tarifários na aplicação própria

Não fixar esses valores como verdade atual. Definir uma fonte oficial verificável (preferencialmente ANEEL e/ou distribuidoras), periodicidade e data de vigência, metadados da tarifa (residencial, impostos/encargos incluídos ou não) e atualização local auditável. Permitir substituição manual e informar claramente que a fatura da residência é a melhor referência individual. Caso não exista uma fonte estadual uniforme, mostrar uma estimativa com aviso em vez de sugerir precisão inexistente.

## Estados e validações de interface observados

- Campo obrigatório visualizado com asterisco: preço do filamento, gramas, tempo, consumo e valor do kWh; escolha do estado aparece sem asterisco.
- Campos opcionais são descritos como opcionais ou podem ficar vazios.
- Minutos aceitam de 0 a 59; quantidade e custos numéricos têm mínimo 0; taxa de falhas e taxas de venda têm máximo 90 na interface observada; quantidade no orçamento tem mínimo 1; desconto tem limite de 0 a 100.
- Selecionar estado mostra confirmação breve de tarifa aplicada.
- Bandeira de energia é adicionada à tarifa base ao clicar nos atalhos.
- A ajuda de consumo orienta informar watts médios reais durante impressão e fornece modelos aproximados.
- A ajuda da tarifa sugere usar a conta de luz e informa que bandeiras são taxa adicional.
- A área de arrastar/soltar não indicou no DOM um filtro de extensão; restrição de arquivo precisa ser descoberta sem enviar arquivos privados.

## Requisitos funcionais para a aplicação futura

### Must have

- Importar G-code localmente de diversos slicers via adaptadores e extrair duração e consumo por material quando disponíveis, mantendo edição manual e indicando origem/confiança de cada valor.
- Calcular o custo de impressão multicolor como soma do custo de cada carretel/material, com edição manual de quantidades e preços.
- Cálculo local dos custos de filamento, energia, desgaste, falhas, trabalho e custos fixos.
- Tarifa elétrica editável, seletor de UF e estrutura de dados tarifários versionada com fonte e vigência.
- Bandeiras tarifárias configuráveis e separadas da tarifa base.
- Configurar embalagem/frete e escolher se o frete é repassado ou incluído no custo.
- Calcular preço com lucro percentual ou valor fixo e taxas sobre a venda; discriminar custo, lucro, preço e margem.
- Validar entradas e explicar erros, inclusive divisões inválidas e metadados de G-code ausentes.
- Executar sem conta e sem enviar G-code ou dados do orçamento para um servidor.
- Exportar orçamento PDF com campos editáveis e sem exigir logotipo.
- Funcionar em Windows; manter decisões de plataforma e empacotamento compatíveis com futura versão macOS.

### Should have

- Perfis editáveis de impressora (potência, preço e vida útil) e de materiais (nome e preço/kg), sempre com edição por peça.
- Histórico local de cálculos, duplicar orçamento e salvar predefinições exportáveis/importáveis.
- Suportar tempo estimado e consumo por material/extrusor, inclusive impressões multicolor, quando os metadados do arquivo permitirem.
- Acessibilidade de teclado, suporte a leitores de tela, unidades explícitas e interface responsiva para telas menores.
- Documentar contribuição, formato de dados, versionamento de tarifas e política de privacidade.

### Could have

- Estimativa de custo de resina e outros processos em uma etapa futura.
- Comparação entre cenário de tarifa manual e média estadual.
- Exportação CSV/JSON do cálculo e impressão de resumo.
- Importação de perfis específicos de fatiadores e suporte a múltiplos idiomas.

## Dados conceituais (sem decidir banco ou framework)

- **Material:** identificador, nome, preço por kg, unidade, data de atualização opcional; cada cálculo pode associar vários carretéis/cores a quantidades separadas.
- **Impressora:** nome/modelo, consumo médio em W, preço de aquisição, vida útil em horas.
- **Tarifa:** UF, concessionária opcional, valor R$/kWh, classificação, fonte, vigência inicial/final, encargos incluídos e data da consulta.
- **Bandeira:** nome, acréscimo R$/kWh, vigência e fonte.
- **Cálculo:** parâmetros de entrada, origem manual/G-code, componentes e valores intermediários, totais, data e versão da fórmula.
- **Orçamento:** referência do cálculo, dados do emissor/cliente, descrição, quantidade, condições, descontos, observações e caminho local do logotipo, se houver.

## Riscos, limites e decisões pendentes

- **Compatibilidade G-code:** metadados e comentários diferem entre fatiadores e versões. Começar com Anycubic Slicer Next para Kobra X e validar candidatos (AC Slicer, PrusaSlicer, Cura, depois outros como Bambu Studio/OrcaSlicer) com amostras próprias antes de declarar suporte.
- **Tarifas:** valores da referência são indicativos e podem ficar desatualizados; mapear fonte confiável, metodologia, impostos e data de vigência antes de publicá-los como dado padrão.
- **Precisão de energia:** watts médios dependem de impressora, material, temperatura, aquecimento e fase da impressão; leitura por estado não substitui medição nem conta de energia.
- **Regras financeiras:** definir arredondamento e tratamento de falhas/taxas, e validar fórmulas com exemplos independentes.
- **Privacidade:** tratar G-code como arquivo potencialmente confidencial; processamento local, retenção e telemetria devem ser explicitados.
- **Orçamento PDF:** decidir geração local, fontes, moeda/localização, dados mínimos e tratamento seguro do logo.
- **Open source:** licença MIT escolhida para este projeto; manter avisos de copyright/licença e revisar termos de bibliotecas, perfis e ativos de terceiros antes de distribuição.
- **Marca e direitos:** adotar nome, interface, ícones, textos e paleta próprios; não reutilizar ativos ou conteúdo da loja de referência.

## Próximas decisões antes de implementar

1. Escolher nome provisório para o aplicativo.
2. Definir a versão do Anycubic Slicer Next e o primeiro arquivo próprio de amostra para Kobra X; depois ordenar adaptadores para outros slicers.
3. Selecionar fonte e método de atualização de tarifas por UF e bandeira.
4. Definir política de armazenamento local (histórico, perfis, logo e orçamentos).
5. Escolher estratégia de empacotamento Windows e macOS, mantendo a interface desacoplada da regra de cálculo.
6. Especificar arredondamentos, entradas inválidas, testes de fórmula e limites de arquivos.
