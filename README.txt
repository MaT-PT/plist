=== PList ===
  Groupe 26
    [AUTEUR 1]
    [AUTEUR 2]


== Compilation ==

Pour compiler plist.exe, il suffit normalement de lancer le script build.bat.
S’il est lancé depuis un terminal de développement de Visual Studio, il lancera
directement cl.exe pour compiler.
Sinon, il cherchera à initialiser automatiquement l’environnement de développement
grâce à l’outil vswhere de Microsoft (inclus dans .\tools\).

En cas de problème, il est possible de compiler plist.exe manuellement en lançant
la commande suivante depuis un terminal de développement de Visual Studio :

cl.exe /O2 /Zi /W4 /Feplist.exe *.c


Le script clean.bat permet de nettoyer les fichiers intermédiaires de compilation.


== Utilisation ==

------------------------------------------------------------------------------------
/!\ ATTENTION /!\
Il faut lancer plist.exe en tant qu’administrateur (terminal avec privilèges élevés)
pour pouvoir afficher les informations sur les processus du système.
Dans le cas contraire, le programme fonctionnera mais il manquera certaines données.
------------------------------------------------------------------------------------

Usage: plist.exe [-h] [-d] [-m] [-e] [name|PID]
    -d          Show thread detail.
    -m          Show memory detail.
    -e          Match process name exactly.
    name        Show information about processes that begin with the name specified.
    PID         Show information about specified process.
    -h          Show this help message and exit.

Abbreviations:
    PID         Process ID
    PPID        Parent Process ID
    Pri         Priority
    Thd         Number of Threads
    Hnd         Number of Handles
    WS          Working Set
    Priv        Private Virtual Memory
    Priv Pk     Private Virtual Memory Peak
    Faults      Page Faults
    NonP        Non-Paged Pool
    Page        Paged Pool


Exemples d’utilisation :
  plist.exe
    Affiche les informations sur tous les processus en cours d’exécution.

  plist.exe -m ex
    Affiche les informations détaillées sur l’utilisation mémoire des processus
    dont le nom commence par « ex ».

  plist.exe -d -e svchost
    Affiche les informations détaillées sur les threads des processus dont le nom
    est exactement « svchost ».

  plist.exe -d 1234
    Affiche les informations détaillées sur les threads du processus dont le PID
    est 1234.
