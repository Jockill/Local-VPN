rm -f *.py;

wget -nv https://git.unistra.fr/alfroy/projet_algo_reseau2021/-/archive/master/projet_algo_reseau2021-master.tar.gz
tar -xf projet_algo_reseau2021-master.tar.gz;
mv projet_algo_reseau2021-master/medium.py .;
rm -rf projet_algo_reseau2021-master projet_algo_reseau2021-master.tar.gz;
