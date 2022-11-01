# slash

# Projet : programmation d'un interpréteur de commandes

**L3 Informatique - Système**

Il est important de bien lire le sujet jusqu'au bout et de bien y réfléchir
avant de se lancer dans la programmation du projet.

## Sujet : `slash`, *small laudable shell*

Le but du projet est de programmer un interpréteur de commandes (aka
*shell*) interactif reprenant quelques fonctionnalités plus ou moins
classiques des shells usuels : outre la possibilité d'exécuter toutes les
commandes externes, `slash` devra proposer quelques commandes internes,
permettre la redirection des flots standard ainsi que les combinaisons
par tube, adapter le prompt à la situation, et permettre l'expansion des
chemins contenant certains jokers décrits ci-dessous.

La gestion des tâches n'est pas requise : tous les processus lancés à
partir de votre shell le seront en premier-plan.

Plus précisément, `slash` doit respecter les prescriptions suivantes : 


### Commandes externes

`slash` peut exécuter toutes les commandes externes, avec ou sans
arguments, en tenant compte de la variable d'environnement `PATH`. 

Pour éviter d'éventuels débordements (notamment lors de l'expansion des
jokers), on pourra limiter le nombre et la taille des arguments autorisés
sur une ligne de commande `slash` :

```c
#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096
```

### Commandes internes

`slash` possède (au moins) les commandes internes suivantes :

##### `exit [val]` 

Termine le processus `slash` avec comme valeur de retour `val` (ou 0 par
défaut).

##### `pwd [-L | -P]`

Affiche la (plus précisément, une) référence absolue du répertoire de travail 
courant :

- avec l'option `-P`, sa référence absolue *physique*, c'est-à-dire ne
  faisant intervenir aucun lien symbolique;

- avec l'option `-L` (option par défaut), sa référence absolue *logique*,
  déduite des paramètres des précédents changements de répertoire
  courant, et contenant éventuellement des liens symboliques.

La valeur de retour est 0 en cas de succès, 1 en cas d'échec.

##### `cd [-L | -P] [ref | -]`

Change de répertoire de travail courant en le répertoire `ref` (s'il
s'agit d'une référence valide), le précédent répertoire de travail si le
paramètre est `-`, ou `$HOME` en l'absence de paramètre.

Avec l'option `-P`, `ref` (et en particulier ses composantes `..`) est
interprétée au regard de la structure physique de l'arborescence.

Avec l'option `-L` (option par défaut), `ref` (et en particulier ses
composantes `..`) est interprétée de manière logique (`a/../b` est
interprétée comme `b`) si cela a du sens, et de manière physique sinon.

La valeur de retour est 0 en cas de succès, 1 en cas d'échec.

##### Exemple (en bash sur lulu) :
```bash
poulalho@lulu:~$ ls -l /tmp/d
lrwxrwxrwx 1 poulalho Enseignants 11 24 oct.  15:55 /tmp/d -> /tmp/a/b/c/
poulalho@lulu:~$ cd /tmp/d
poulalho@lulu:/tmp/d$ pwd
/tmp/d
poulalho@lulu:/tmp/d$ pwd -P
/tmp/a/b/c
poulalho@lulu:/tmp/d$ cd .. ; pwd            # interprétation logique : /tmp/d/.. = /tmp
/tmp
poulalho@lulu:/tmp$ cd - ; pwd
/tmp/d
poulalho@lulu:/tmp/d$ cd -P .. ; pwd         # interprétation physique : /tmp/d/.. = /tmp/a/b/c/.. = /tmp/a/b
/tmp/a/b
poulalho@lulu:/tmp/d$ cd - ; pwd
/tmp/d
poulalho@lulu:/tmp/d$ cd ../../../d ; pwd    # /tmp/d/../../../d n'a pas de sens => interprétation physique
/tmp/a/b/c
```

### Gestion de la ligne de commande

`slash` fonctionne en mode interactif : il affiche une invite de commande
(*prompt*), lit la ligne de commande saisie par l'utilisateur, la découpe
en mots selon les (blocs d')espaces, interprète les éventuels caractères
spéciaux (`<`, `>`, `|`, `*`, cf ci-dessous), puis exécute la ou les
commandes correspondantes.

`slash` ne fournit aucun mécanisme d'échappement; les arguments de la
ligne de commande ne peuvent donc pas contenir de caractères spéciaux
(espaces en particulier). 

Il est fortement recommandé d'utiliser la bibliothèque
[`readline`](https://tiswww.cwru.edu/php/chet/readline/rltop.html) pour
simplifier la lecture de la ligne de commande : cette bibliothèque offre
de nombreuses possibilités d'édition pendant la saisie de la ligne de
commande, de gestion de l'historique, de complétion automatique... Sans
entrer dans les détails, un usage basique de la fonction `char *readline
(const char *prompt)` fournit déjà un résultat très satisfaisant :

```c
char * ligne = readline("mon joli prompt $ ");  
/* alloue et remplit ligne avec une version "propre" de la ligne saisie,
 * ie nettoyée des éventuelles erreurs et corrections (et du '\n' final) */
add_history(ligne);
/* ajoute la ligne à l'historique, permettant sa réutilisation avec les
 * flèches du clavier */
```

L'affichage du prompt de `slash` est réalisé **sur sa sortie erreur**.
Avec la bibliothèque `readline`, ce comportement s'obtient en indiquant
`rl_outstream = stderr;` avant le premier appel à la fonction
`readline()`.


#### Formatage du prompt

Le prompt est limité à 30 caractères (apparents, ie. sans compter les
indications de couleur), et est formé des éléments suivants :

- un code de bascule de couleur, `"\033[32m"` (vert) ou `"\033[91m"`
  (rouge) suivant que la dernière commande exécutée a réussi ou échoué;
- entre crochets, la **valeur de retour** de la dernière commande
  exécutée (0 par défaut pour le premier prompt); 
- une autre bascule de couleur, par exemple `"\033[34m"` (bleu) ou
  `"\033[36m"` (cyan);
- la référence (logique) du **répertoire courant**, éventuellement
  tronquée (par la gauche) pour respecter la limite de 30 caractères;
  dans ce cas la référence commencera par trois points (`"..."`);  
- la bascule `"\033[00m"` (retour à la normale);  
- un dollar puis un espace (`"$ "`).  

Par exemple (sans la coloration) :
```bash
[0]/home/titi$ cd pas/trop/long
[0]/home/titi/pas/trop/long$ cd mais/la/ca/depasse
[0]...ong/mais/la/ca/depasse$ pwd
/home/titi/pas/trop/long/mais/la/ca/depasse
[0]...ong/mais/la/ca/depasse$ cd repertoire-inexistant
cd: no such file or directory: repertoire-inexistant
[1]...ong/mais/la/ca/depasse$ 
```

### Jokers

`slash` réalise l'expansion des jokers suivants :

- `*` : tout **préfixe** (ne commençant pas par `'.'`) d'un **nom de
  base** (valide, donc en particulier non vide);

- `**/` : tout préfixe de chemin **physique** de la forme `*/*/.../`
  formé d'au moins une composante (en particulier, chaque composante 
  est non vide, il s'agit donc de références relatives au répertoire
  courant);

Par exemple :

- `echo A/B/C/*` affiche les références de tous les fichiers (non cachés)
  du répertoire `A/B/C`

- `echo A/B/C/*.c` affiche les références de tous les fichiers (non
  cachés) d'extension `.c` du répertoire `A/B/C` 

- `echo **/toto` affiche les références de tous les fichiers de nom de
  base `toto` dans la partie non cachée de l'arborescence courante

- `echo **/*.c` affiche les références de tous les fichiers d'extension
  `.c` dans la partie non cachée de l'arborescence courante

Seule l'expansion de ces jokers en tant que **préfixe** est requise;
l'expansion d'expressions telles que `toto*tutu`, ou `*/A/**/toto` n'est
pas exigée.


### Redirections

`slash` gère les redirections suivantes :

- `cmd < fic` : redirection de l'entrée standard de la commande `cmd` sur
  le fichier (ordinaire, tube nommé...) `fic`

- `cmd > fic` : redirection de la sortie standard de la commande `cmd`
  sur le fichier (ordinaire, tube nommé...) `fic` **sans écrasement**
  (ie, échoue si `fic` existe déjà)

- `cmd >| fic` : redirection de la sortie standard de la commande `cmd`
  sur le fichier (ordinaire, tube nommé...) `fic` **avec écrasement**
  éventuel

- `cmd >> fic` : redirection de la sortie standard de la commande `cmd`
  sur le fichier (ordinaire, tube nommé...) `fic` **en concaténation**

- `cmd 2> fic` : redirection de la sortie erreur standard de la commande
  `cmd` sur le fichier (ordinaire, tube nommé...) `fic` **sans
  écrasement** (ie, échoue si `fic` existe déjà)

- `cmd 2>| fic` : redirection de la sortie erreur standard de la commande
  `cmd` sur le fichier (ordinaire, tube nommé...) `fic` **avec
  écrasement** éventuel

- `cmd 2>> fic` : redirection de la sortie erreur standard de la commande
  `cmd` sur le fichier (ordinaire, tube nommé...) `fic` **en
  concaténation**

- `cmd1 | cmd2` : redirection de la sortie standard de `cmd1` et de
  l'entrée standard de `cmd2` sur un même tube anonyme

Les espaces de part et d'autre des symboles de redirection sont requis.

Par ailleurs, dans un *pipeline* `cmd1 | cmd2 | ... | cmdn`, les
redirections additionnelles sont autorisées :

- redirection de l'entrée standard de `cmd1` 
- redirection de la sortie standard de `cmdn`
- redirection des sorties erreurs des `n` commandes


### Sensibilité aux signaux

`slash` ignore les signaux `SIGINT` et `SIGTERM`, contrairement aux
processus exécutant des commandes externes.


## Modalités de réalisation (et de test)

Le projet est à faire par équipes de 3 étudiants, exceptionnellement 2.
La composition de chaque équipe devra être envoyée par mail à
l'enseignante responsable du cours de systèmes avec pour sujet `[SY5]
équipe projet` **au plus tard le 10 novembre 2022**, avec copie à chaque
membre de l'équipe.

Chaque équipe doit créer un dépôt `git` **privé** sur le [gitlab de
l'UFR](https://gaufre.informatique.univ-paris-diderot.fr/) **dès la
constitution de l'équipe** et y donner accès en tant que `Reporter` à
tous les enseignants du cours de Système : Mirna Džamonja, Isabelle
Fagnot, Guillaume Geoffroy, Anne Micheli et Dominique Poulalhon. Le dépôt
devra contenir un fichier `AUTHORS.md` donnant la liste des membres de
l'équipe (nom, prénom, numéro étudiant et pseudo(s) sur le gitlab).

En plus du code source de votre programme, vous devez fournir un
`Makefile` tel que :
  - l'exécution de `make` à la racine du dépôt crée (dans ce même
    répertoire) l'exécutable `slash`,
  - `make clean` efface tous les fichiers compilés,

ainsi qu'un fichier `ARCHITECTURE.md` expliquant la stratégie adoptée
pour répondre au sujet (notamment l'architecture logicielle, les
structures de données et les algorithmes implémentés).

**Le projet doit s'exécuter correctement sur lulu**.

Les seules interdictions strictes sont les suivantes : plagiat (d'un
autre projet ou d'une source extérieure à la licence), utilisation de
la fonction `system` de la `stdlib` et des fonctions de `stdio.h`
manipulant le type `FILE` pour les redirections.

Pour rappel, l'emprunt de code sans citer sa source est un
plagiat. L'emprunt de code en citant sa source est autorisé, mais bien
sûr vous n'êtes notés que sur ce que vous avez effectivement produit.
Donc si vous copiez l'intégralité de votre projet en le spécifiant
clairement, vous aurez quand même 0 (mais vous éviterez une demande de
convocation de la section disciplinaire).

## Modalités de rendu

Le projet sera évalué en 3 phases : deux jalons intermédiaires, sans
soutenance, et un rendu final avec soutenance. Les deux jalons
intermédiaires seront évalués par des tests automatiques.

### Premier jalon jeudi 24 novembre

La version à évaluer devra être spécifiée à l'aide du tag `jalon-1`. 

Pour créer le tag : se placer sur le commit à étiqueter et faire :
```
git tag jalon-1
git push origin --tags
```

Points testés :

- existence du dépôt git
- présence du fichier `AUTHORS.md`
- compilation sans erreur avec `make` à la racine du dépôt
- exécution de `slash` à la racine du dépôt
- bon fonctionnement de `exit`, `cd`, `pwd`
- conformité du prompt (hors valeur de retour)

### Deuxième jalon jeudi 15 décembre

La version à évaluer devra être spécifiée à l'aide du tag `jalon-2`.

Points testés : ceux du premier jalon, plus :

- exécution des commandes externes (avec arguments mais sans redirections)
- adaptation du prompt selon la valeur de retour
- expansion de l'étoile simple (`'*'`)


### Rendu final début janvier

La version à évaluer devra être spécifiée à l'aide du tag `rendu-final`.

Le projet final devra être rendu à une date encore à définir mais au plus
tard le 8 janvier, pour des soutenances entre le 9 et le 13 janvier.


### Participation effective

**Les projets sont des travaux de groupe, pas des travaux à répartir
entre vous** ("je fais le projet de Prog fonctionnelle, tu fais celui de
Systèmes"). La participation effective d'un étudiant au projet de son
groupe sera évaluée grâce, entre autres, aux statistiques du dépôt git et
aux réponses aux questions pendant la soutenance, et les notes des
étudiants d'un même groupe pourront être modulées en cas de participation
déséquilibrée. 

**En particulier, un étudiant n'ayant aucun commit sur les aspects
réellement "système" du code et incapable de répondre de manière
satisfaisante sera noté "DEF".**


