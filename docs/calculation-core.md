# Núcleo de cálculo local

O cálculo usado pelo aplicativo está em `src/calculation/calculate_cost.h` e `src/calculation/calculate_cost.cpp`. A função C++ `calculation::calculateCost(input)` não acessa arquivos, rede, interface nem catálogo de tarifas; recebe valores numéricos explícitos e retorna a discriminação por peça em `calculation::CostBreakdown`. O protótipo exploratório em `src/calculation/calculate-cost.mjs` permanece como referência.

## Entradas e formulário

- Uma ou mais entradas de material com `grams` e `pricePerKg`.
- `printTimeSeconds`, `powerWatts`, `electricityRatePerKwh` e `electricityFlagPerKwh`.
- `printerPurchasePrice` e `printerLifeHours` para desgaste; devem ser informados juntos.
- `failureRatePercent`, `laborCost`, `fixedCost` e `packagingCost`.
- `shippingCost` e `shippingMode`: `none`, `pass-through` ou `cost`.
- `profitMode` (`percent` ou `fixed`), `profitValue` e `salesFeePercent`.

A tela em `src/app/main_window.cpp` permite preencher os dados manualmente, acrescentar/remover materiais e editar valores enquanto o resultado se atualiza. Cada material precisa de gramas e preço por quilo; potência e tarifa também são necessárias. O tempo começa em zero e pode ser ajustado em horas e minutos. A bandeira e todos os campos adicionais podem ficar vazios, que significa zero. Frete preenchido exige a escolha de como aplicá-lo. O nome de cada material na interface indica a sua ordem na lista.

Campos numéricos aceitam vírgula ou ponto como separador decimal. Quando os dois aparecem, o último é interpretado como separador decimal e o outro como separador de milhar. Os campos de percentuais também aceitam o símbolo `%` ao final, por exemplo `30%`. Unidades e exemplos aparecem nos campos.

## Cálculos

- Filamento: soma de `(gramas do material ÷ 1000) × preço por kg`.
- Energia: `(watts × duração em horas ÷ 1000) × (tarifa base + bandeira)`.
- Desgaste: `(preço da impressora ÷ vida útil em horas) × duração em horas`.
- Falhas: `(filamento + energia + desgaste) × percentual ÷ (100 − percentual)`.
- Custo: filamento + energia + desgaste + falhas + mão de obra + custos fixos + embalagem; frete entra aqui se `shippingMode` for `cost`.
- Lucro: percentual do custo ou valor fixo por peça.
- Taxas: calculadas sobre o preço antes do frete repassado.
- Preço antes do frete repassado: `(custo + lucro) ÷ (1 − taxas de venda)`.
- Preço final: preço antes do frete repassado + frete, quando `shippingMode` for `pass-through`.
- Margem exibida: lucro ÷ preço final × 100.

O módulo mantém o limite de 0–90% para falhas e taxas. Entradas negativas, não finitas, modos desconhecidos, campos de desgaste preenchidos pela metade e frete sem modo lançam erros com mensagens em português. Valores monetários são arredondados a centavos ao montar os resultados; os intermediários usam `double`.

## Limitações

O núcleo C++ está conectado ao formulário e o cálculo atualiza enquanto os campos são editados. O parser importa tempo e dados de filamento reconhecidos nos metadados G-code; campos ausentes podem ser preenchidos manualmente. Ainda não foram adicionados testes automatizados para as fórmulas, e a compatibilidade precisa ser validada com mais amostras. A tela não fornece perfis de materiais/impressora ou persistência. O usuário deve conferir a tarifa de energia aplicada.
