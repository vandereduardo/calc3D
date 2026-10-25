# Núcleo de cálculo local

Protótipo em `src/calculation/calculate-cost.mjs`. A função `calculateCost(input)` não acessa arquivos, rede, interface nem catálogo de tarifas. Ela recebe todos os valores explicitamente e retorna um detalhamento por peça. Isso permite integrar o cálculo a uma interface desktop depois de escolher a tecnologia.

## Entradas atuais

- `materials`: lista de `grams` e `pricePerKg`, podendo incluir nome e cor. Cada material é calculado separadamente e os custos são somados.
- `printTimeSeconds`, `powerWatts`, `electricityRatePerKwh` e `electricityFlagPerKwh`.
- `printerPurchasePrice` e `printerLifeHours` para desgaste; devem ser informados juntos.
- `failureRatePercent`, `laborCost`, `fixedCost` e `packagingCost`.
- `shippingCost` e `shippingMode`: `none`, `pass-through` ou `cost`.
- `profitMode` (`percent` ou `fixed`), `profitValue` e `salesFeePercent`.

## Cálculos

- Filamento: soma de `(gramas do material ÷ 1000) × preço por kg`.
- Energia: `(watts × duração em horas ÷ 1000) × (tarifa base + bandeira)`.
- Desgaste: `(preço da impressora ÷ vida útil em horas) × duração em horas`.
- Falhas: `(filamento + energia + desgaste) × percentual ÷ (100 − percentual)`.
- Custo: filamento + energia + desgaste + falhas + mão de obra + custos fixos + embalagem; frete entra aqui se `shippingMode` for `cost`.
- Lucro: percentual do custo ou valor fixo por peça.
- Preço antes de frete repassado: `(custo + lucro) ÷ (1 − taxas de venda)`.
- Frete em modo `pass-through` é acrescentado ao preço depois do gross-up, sem lucro nem taxas.

O módulo segue os limites de 0–90% observados para falhas e taxas no formulário de referência. As entradas inválidas lançam erro com uma mensagem em português. Valores monetários são arredondados para centavos apenas na saída; os cálculos intermediários mantêm precisão decimal de JavaScript.

## Estado e limitações

Este é um protótipo ainda não validado. Não inclui dados tarifários das UFs, perfis de impressora/material, persistência ou interface. O MVP apresentará o cálculo na própria tela e não gerará PDF. O chamador precisa fornecer uma tarifa atual e verificável. Antes de integrar ao app, revisar arredondamento, margem real com frete repassado, limite de falhas, dados incompletos e todos os casos de erro.
