# Como contribuir

Obrigado pelo interesse no calc3D. O projeto está em planejamento; leia o [README](README.md), o [reconhecimento funcional](replica/recon.md) e as [decisões pendentes](docs/decisions.md) antes de propor mudanças.

## Antes de abrir uma issue

- Para relatar um problema, inclua o que esperava, o que aconteceu e os passos para reproduzir.
- Para sugerir funcionalidade, explique o usuário, a necessidade e o resultado esperado.
- Não anexe G-code de clientes, dados pessoais, contas de energia identificáveis ou outros arquivos privados. Use exemplos sintéticos ou anonimizados.
- Para valores de tarifa, inclua fonte pública, unidade, área atendida e período de validade.

## Pull requests

1. Abra uma issue para discutir mudanças grandes antes de investir na implementação.
2. Faça uma alteração focada e explique sua motivação.
3. Atualize a documentação quando uma regra de cálculo, fonte de dados ou fluxo mudar.
4. Inclua ou atualize testes automatizados quando a base de código e a estratégia de testes estiverem definidas.
5. Informe como verificou a mudança e quais limitações ainda existem.

O fluxo de branches, convenção de commits, comandos de desenvolvimento e verificações obrigatórias serão detalhados após a escolha da tecnologia. As contribuições aceitas serão distribuídas sob a licença MIT deste repositório; ao enviar uma contribuição, confirme que tem o direito de submetê-la sob esses termos.

## Dados e privacidade

G-code pode conter informações sobre uma peça ou processo de fabricação. Não publique arquivos reais de usuários. Prefira amostras sintéticas, pequenas e sem comentários que revelem dados privados. A aplicação deverá processar esses arquivos localmente por padrão.
