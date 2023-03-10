# Architecture de slash


## Structures de données

Nous avons implémenté un lexer, un parser,  des jetons (token) , des tableaux à taille variable (vector.c), 
des chaînes de caractères à taille variable, un arbre abstrait de syntaxe et un automate (automaton.c).


## Les modules

### Jokers (wildcard.c) 

Le module possède une structure interne les wildcard\_token qui ont
un string path gardant leur chemin, un type entre `PATH` et `REGEX`, un vecteur de sous-jeton,
et un entier is\_dir qui indique si le token représente un répertoire.

Le module possède une fonction principale qui prend en paramètre un jeton, 
dont le contenu sera étendu. Cette fonction réalise differents appels :


1. un lexer interne qui se chargera de détecter `*`,
`**`,`$`, `~`, `/`, `?`, `[`, `]`, `-`, 
ou des caractères autres. Il met les différents jetons dans un vecteur.

2. un parser qui recupère les jetons du lexer et qui crée des wildcard\_token dans un vecteur.
Il coupe le phrase par slash, si le mot contient un joker, alors le type sera un 
`REGEX` sinon ce sera un `PATH`. 

Dans les REGEX, le vector subtokens permet de représenter le mot.
Il est composé de tokens `STAR` pour `*`, `DSATR` pour `**`, `SOME` pour `[ quelque chose ]`, 
ARG pour les autres caractères (lettres, points, etc).

3. Un interpreteur qui s'occupera de faire la recherche depuis la liste de wildcard\_tokens.


Pour la recherche, on réalise un dfs itératif.
Si le mot actuel est une double étoile, 
alors on ajoute un token étoile gardant la même propriété vis-à-vis des répertoires.

On parcours le repertoire courant en ommetant `.` et `..`. 

On vérifie pour chaque nom qu'il est bien reconnu par l'automate, si c'est le cas, 
on l'ajoute à un vecteur temporaire. Si on est dans le cas d'une double étoile,
on l'ajoute à un deuxième vecteur temporaire.

Ensuite, on parcours le vecteur du double étoile, si le fichier existe et 
que ce n'est pas un lien, on l'ajoute à notre pile de répertoire à visiter.

Puis, on parcours les fichiers qui ont été reconnus par l'automate,
si ce sont des répertoire on les ajoutes à la pile, 
sinon on regarde si on est à la fin du mot. 
Si c'est le cas, on les ajoutes au résultat final.


### Automate (automaton.c)

Les automates sont utilisés pour l'expansion des jokers. 

Ils utilisent un vecteur de jetons pour se créer.

Pour la reconnaissance, on parcours le mot carcatère par caractère.
Si un état possède plusieurs liens et que le caractère est reconnu par plusieurs lien, 
on les ajoutes à un vecteur de liens valides, 
et on prend le dernier.

On avance ainsi d'état en état jusqu'à la fin du mot.

Le mot est reconnu si, et seulement si, on est sur un état acceptant à la fin du mot.

Pour la vérifications des liens, si le lien est de type :

* `STAR` : tout carcatère est reconnu

* `SOME` : le jeton était de la forme `[x-y]` (`[xy]` <=> `[x-xy-y]`), et on reconnait 
tout caractère avec un code ascii z tel que x <= z <= y ou y <= z <= x

* `SPEC_NONE` : on compare un morceau du mots a vérifier avec celui du lien. S'ils sont 
égaux, alors le morceau est reconnu

* `QUESTION_MARK` : On reconnait tout caractère sauf le caractère vide.


### Commandes internes (internals.c, pwd.c, cd.c, exit.c)

#### `internals.c`

Regroupe les différentes commandes internes dans un tableau, 
ce qui permet de vérifier facielement si une commande donnée est interne.

Pour des raisons de simplification de redirection, la déclaration des commandes internes 
et la même que celle des commandes externes, 
i.e. celle d'un main avec les descripteurs de l'entrée standard, 
de la sortie standard et de l'erreur standard.

#### `pwd.c`

Fait un appel à ```$PWD``` pour afficher les chemin courant avec les options ```-L``` (Logical) et ```-P``` (physical).

la commande accepte plusieus arguments. C'est-à-dire, ```pwd -L -P -P -L``` sera accepté et prendra comme paramètre principal le dernier argument (dans l'exemple ```-L```).

Retourne ```0``` si le l'executation réussi ou si les lien ne sont pas cassés, et `1` en cas d'erreur.

#### `cd.c`

Fait un appel aux variables d'environment ```$HOME``` et ```$PWD``` pour changer de repertoire. Les options ```-L``` et ```-P``` sont acceptées en plus du chemin.

La commande normalise le chemin passé en argument et met à jour la variable d'environment ```$PWD```.

Retourne ```0``` si le chemin existe bien et ```1``` dans les autres cas.

#### `exit.c`
Notifie le programme qu'il doit quitter à la fin de la lecture de la phrase.

### Errno (slasherrno.c)
Variable qui stocke la valeur de retour des fonctions critiques, des commandes, etc. 

Cette variable est utilisée dans l'affichage de message d'erreur mais aussi du code d'erreur.

### Lexer (lexer.c)

Découpe le la ligne de commande selon les espaces. 

Le lexer reconnait toutes les redirections, les commandes selon leur emplacement, 
`"`,`'`, `!`, `;`, `if`, `for`, `then`, 
`do`,`else`, `done`,`in` , '&&', '||' et les autres carcatères. 

Il crée les jetons correspondant et les met dans un vecteur de jetons.

### Parser (parser.c)

Le parser lit les jetons produit par le lexer.

Il vérifie que la phrase est correcte.

Il lance l'expansion des jokers si un mot en possède.

Il vérifie si une commande est interne ou externe.

Si il y a un `"` ou un `'`, il regroupe les jetons entre les `"` et les `'` dans un même jeton `ARG`.

Les guillements ou les apostrophes contenues dans un même mot ne sont pas prises en compte.

Si le mot contient un variable, il la cherche dans l'environnement et réalise son expansion.

### Main (slash.c)

Affiche le prompt depuis la variable $PWD et la valeur de slasherrno

Pour chaque entrée, on appelle le lexer sur la ligne. 
	
Puis on appelle le parser sur le vecteur de token

Puis on execute la ligne grâce à l'ast

Si on a un appel à exit, on quitte et on affiche le message de sortie.

### AST (ast.c)

Récupère la ligne de jetons créée par le parser lors de la récupération d'une ligne sur l'entrée standard.

Pour simplifier le travail, une polonaise inversée est appliquée sur la chaîne de jetons.

Grâce à cette dernière, il est facile de construire l'arbre.

Localement, les commande et opérateur représentent les noeuds tandis que les arguments représentent les feuilles.

En présence de tubes dans l'expression donnée, plusieurs arbres sont créés.

Lors de l'exécution, ils seront exécutés chacun par un processus différent.

### Structures de donnée

De manière à rendre l'abstraction plus facile, plusieurs structures ont été implémentées, suivant un modèle orienté objet.

Ainsi, le vecteur (vector.c) représente un tableau dynamique, qui effectue chaque ajout par recopie.

La chaîne de caractère dynamique (string.c) est basée sur le vecteur, étant un cas particulier d'un tableau dynamique de caractère.

Des fonctions utilitaires ont pu être écrite ensuite, telles que la troncation, la comparaison, la récupération etc.

### Gestion de signaux

De manière à detecter les signaux réçues, on utilise deux variables "globales": ```interrupt_state``` et ```sigterm_received``` (signal.h).

Par défaut elles sont toutes les deux initialisée à ```0```. Elles prennent la valeur ```1``` si les signaux ```SIGINT``` OU ```SIGTERM``` sont captés.

On vérifie dans l'intégralité du code la valeur de ces deux variables afin de arreter les processus en cas de signal réçu.