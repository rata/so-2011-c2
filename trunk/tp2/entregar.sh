
rm -f tp2.tar.gz
rm -rf entregar
mkdir -p entregar
cd src/backend-mt/
make clean
cd -
cp -r src/backend-mt/* entregar/
cp informe/informe.pdf entregar/
tar czf tp2.tar.gz entregar/
