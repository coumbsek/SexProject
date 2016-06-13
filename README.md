**GUIDE DE L’UTILISATEUR**

<p>1-	Après avoir récupéré l’archive utiliser la commande suivante pour la décompresser :
		**tar xvf BERNARD.LILA.G3.tar**</p>

<p>2-	Déplacez-vous dans le dossier module/ via la commande : **cd module/**
Utilisez la commande make pour générer la librairie.</p>

<p>3-	Déplacez-vous dans le dossier src/ via la commande : **cd ../src/**
Utilisez la commande make pour générer les exécutables.</p>

<p>4-	Rendez-vous désormais dans le dossier bin via la commande : **cd ../bin/**</p>

5-	

<p>Pour lancer l’annuaire tapez : **./annuaire**
		Celui-ci utilise le port 8080 par défaut, assurez-vous donc qu’il soit libre. Vous pouvez toujours le changer en éditant le fichier constantes.h qui se trouve dans le dossier src/ et en répétant les opérations 3 à 5.</p>

<p>Pour lancer un serveur et donc partager un fichier tapez : 
**./serveur <numéro de port> <fichier à partager>**
				Le fichier à partager doit  se trouver dans ce même dossier.</p>

<p>Pour lancer un client et rejoindre un fichier partagé :
o	Si vous connaissez le serveur tapez : **./client <adresse I.P.> <port>**
o	Si vous souhaitez vous connecter à l’annuaire : **./client**</p>


Exemples et erreurs : 
		Si vous n’avez pas accès à l’annuaire le serveur sera quand même utilisable à condition de connaitre son adresse I.P. pour la connaître sur un réseau local tapez **ifconfig**
		
		$ ./server 3000 example.txt
		$ ifconfig**
			inet addr:192.168.243.132
		Si vous souhaitez vous connecter directement utilisez la forme à 3 arguments du client
		$ ./client 192.168.243.132 3000
