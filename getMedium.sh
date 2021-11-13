rm -f *.py;

if [ "$1" = "PROF" ]
then
        wget -nv https://git.unistra.fr/alfroy/projet_algo_reseau2021/-/archive/master/projet_algo_reseau2021-master.tar.gz
        tar -xf projet_algo_reseau2021-master.tar.gz;
        mv projet_algo_reseau2021-master/medium.py .;
        rm -rf projet_algo_reseau2021-master projet_algo_reseau2021-master.tar.gz;
else
        wget -nv https://git.unistra.fr/pvcmeyer/projet_algo_reseau2021/-/raw/master/medium.py
fi
