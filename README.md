// ------------------------------------------------------
	Démarrer avec goblim - WINDOWS
// ------------------------------------------------------
- Lancer la solution GoblimSolution/GoblimGL4.sln
- Définissez le projet SampleProject comme "Projet de démarrage" (click droit sur le projet - définir comme projet de démarrage)
- Dans la section "dépendance de build" du menu du projet, ajouter le projet GoblimGL4 comme dépendance de build

- Si ça ne marche pas : 
-- il se peut que des chemin d'inclusion ou de lib se soit remplacer à l'ouverture de la solution en dur (mais j'espère que non)
-- La plateforme AMD est en test, le suppot intel HD n'est pas encore prévu

// ------------------------------------------------------
	Démarrer avec goblim - LINUX
// ------------------------------------------------------
- Dans le répertoire Libraries/ToCompileForLinux : lisez rapidement le readme et installez les librairie nécessaires
- Lancer la solution LinuxSublimeSolution/Goblim.sublime-project
- Dans Goblim Solution/Makefile : Modifier la variable project pour qu'elle corresponde au projet à compiler (pas de version multi projet pour le moment)
- Vous pouvez tester la compilation du moteur lui même en faisant "make goblim -j 8" (-j lance la compilation sur plusieurs thread, ici 8)
- Pour compiler le projet, vous pouvez utilisez simplement "make", ou utilisez les options de build fournis dans le projet sublime text (ctrl+b)
- Sous linux, toute l'édition/compilation du projet/mise a jour des shaders peut être faite directement depuis le projet sublime-text situé dans LinuxSublimeSolution
- Je vous conseil de regarder quelles options de build sont disponibles


// ------------------------------------------------------
	Ajouter votre propre projet
// ------------------------------------------------------
- Copier le dossier SampleProject et renommé le dossier et les fichiers de visual studio et le fichier sublime-project
-- Attention a bien faire correspondre les noms de vos projet sublime text et visual pour que le déploiement des shaders par sublime text fonctionne
- Dans la solution de visual studio, ajouter votre projet à la solution
- dans la section "dépendance de build" du menu du projet, ajouter le projet GoblimGL4 comme dépendance de votre projet
- Définissez le projet comme "Projet de démarrage"
- lancer la compilation

// ------------------------------------------------------
	Modifier le répertoire de sortie - WINDOWS
// ------------------------------------------------------
Pour modifier le répertoire de sortie de votre projet, il faut modifier 4 choses :
- Les répertoire de sortie et intermédiaire du projet goblimGL4
- Les répertoire de sortie et intermédiaire de votre projet
- Les répertoires de librairie de votre projet 
-- C:\Home\Output\GobLimGL4\$(Configuration) est à remplacer par votre nouveau répertoire de sortie du projet GoblimGL4
- Les répertoires de sortie configuré dans les "build_systems" du fichier sublime-project
-- Pour modifier le fichier, ouvrez un sublime-text et glissez-déposez le fichier dans l'éditeur, sinon il vous ouvrira le projet au lieu du fichier
 
// ------------------------------------------------------
	Mise a jour des shaders - WINDOWS
// ------------------------------------------------------
Ce projet déploiement rend le debuggage de shader un peu plus long si vous n'utilisez que visual studio
- Les shaders utilisés par le moteur au débuggage sont une copie de ceux de votre projet
	-- Ils sont copiés à la génération dans le C:/Home/OutPut/<VotreProjet+config>/Materials ou Effects
 - Cette copie n'est mise a jour qu'a chaque régénération du projet par visual studio

################ EN UTILISANT SUBLIME TEXTE #########################
- Ouvrer le projet <nomdevotreprojet>.sublime-project
- Modifier vos shaders depuis sublime texte
- Compiler avec l'option de build qui vous convient (tools->build system->transfert GLSL to debug/release)
- relancer votre exe depuis visual OU lancer directement l'exe depuis son répertoire de sortie

################ DIRECTEMENT VISUAL STUDIO #########################
Pour la mise à jour des shaders situés dans votre projets, il est nécessaire de regénérer votre projet
Pour aller plus vite, vous pouvez au choix :
- faire clic droit sur le projet -> Projet uniquement -> régénérer le projet pour transférer vos shaders du projet mis a jour

- éditer directement les materiaux situés dans le répertoire de sortie mais attention :
	-- Ces shaders ne sont pas sauvegardé dans le projet
	-- Ces shaders sont éffacés à chaque régénération du projet

