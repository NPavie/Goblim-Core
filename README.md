// ------------------------------------------------------
	D�marrer avec goblim - WINDOWS
// ------------------------------------------------------
- Lancer la solution GoblimSolution/GoblimGL4.sln
- D�finissez le projet SampleProject comme "Projet de d�marrage" (click droit sur le projet - d�finir comme projet de d�marrage)
- Dans la section "d�pendance de build" du menu du projet, ajouter le projet GoblimGL4 comme d�pendance de build

- Si �a ne marche pas : 
-- il se peut que des chemin d'inclusion ou de lib se soit remplacer � l'ouverture de la solution en dur (mais j'esp�re que non)
-- La plateforme AMD est en test, le suppot intel HD n'est pas encore pr�vu

// ------------------------------------------------------
	D�marrer avec goblim - LINUX
// ------------------------------------------------------
- Dans le r�pertoire Libraries/ToCompileForLinux : lisez rapidement le readme et installez les librairie n�cessaires
- Lancer la solution LinuxSublimeSolution/Goblim.sublime-project
- Dans Goblim Solution/Makefile : Modifier la variable project pour qu'elle corresponde au projet � compiler (pas de version multi projet pour le moment)
- Vous pouvez tester la compilation du moteur lui m�me en faisant "make goblim -j 8" (-j lance la compilation sur plusieurs thread, ici 8)
- Pour compiler le projet, vous pouvez utilisez simplement "make", ou utilisez les options de build fournis dans le projet sublime text (ctrl+b)
- Sous linux, toute l'�dition/compilation du projet/mise a jour des shaders peut �tre faite directement depuis le projet sublime-text situ� dans LinuxSublimeSolution
- Je vous conseil de regarder quelles options de build sont disponibles


// ------------------------------------------------------
	Ajouter votre propre projet
// ------------------------------------------------------
- Copier le dossier SampleProject et renomm� le dossier et les fichiers de visual studio et le fichier sublime-project
-- Attention a bien faire correspondre les noms de vos projet sublime text et visual pour que le d�ploiement des shaders par sublime text fonctionne
- Dans la solution de visual studio, ajouter votre projet � la solution
- dans la section "d�pendance de build" du menu du projet, ajouter le projet GoblimGL4 comme d�pendance de votre projet
- D�finissez le projet comme "Projet de d�marrage"
- lancer la compilation

// ------------------------------------------------------
	Modifier le r�pertoire de sortie - WINDOWS
// ------------------------------------------------------
Pour modifier le r�pertoire de sortie de votre projet, il faut modifier 4 choses :
- Les r�pertoire de sortie et interm�diaire du projet goblimGL4
- Les r�pertoire de sortie et interm�diaire de votre projet
- Les r�pertoires de librairie de votre projet 
-- C:\Home\Output\GobLimGL4\$(Configuration) est � remplacer par votre nouveau r�pertoire de sortie du projet GoblimGL4
- Les r�pertoires de sortie configur� dans les "build_systems" du fichier sublime-project
-- Pour modifier le fichier, ouvrez un sublime-text et glissez-d�posez le fichier dans l'�diteur, sinon il vous ouvrira le projet au lieu du fichier
 
// ------------------------------------------------------
	Mise a jour des shaders - WINDOWS
// ------------------------------------------------------
Ce projet d�ploiement rend le debuggage de shader un peu plus long si vous n'utilisez que visual studio
- Les shaders utilis�s par le moteur au d�buggage sont une copie de ceux de votre projet
	-- Ils sont copi�s � la g�n�ration dans le C:/Home/OutPut/<VotreProjet+config>/Materials ou Effects
 - Cette copie n'est mise a jour qu'a chaque r�g�n�ration du projet par visual studio

################ EN UTILISANT SUBLIME TEXTE #########################
- Ouvrer le projet <nomdevotreprojet>.sublime-project
- Modifier vos shaders depuis sublime texte
- Compiler avec l'option de build qui vous convient (tools->build system->transfert GLSL to debug/release)
- relancer votre exe depuis visual OU lancer directement l'exe depuis son r�pertoire de sortie

################ DIRECTEMENT VISUAL STUDIO #########################
Pour la mise � jour des shaders situ�s dans votre projets, il est n�cessaire de reg�n�rer votre projet
Pour aller plus vite, vous pouvez au choix :
- faire clic droit sur le projet -> Projet uniquement -> r�g�n�rer le projet pour transf�rer vos shaders du projet mis a jour

- �diter directement les materiaux situ�s dans le r�pertoire de sortie mais attention :
	-- Ces shaders ne sont pas sauvegard� dans le projet
	-- Ces shaders sont �ffac�s � chaque r�g�n�ration du projet

