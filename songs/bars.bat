# bars.bat
mkdir "01-03- (It Mustve Been Ol) Santa Claus"
gawk -f bars4.awk -v filename="01-03- (It Mustve Been Ol) Santa Claus_bars.txt" "01-03- (It Mustve Been Ol) Santa Claus_bars.txt"
move /Y "01-03- (It Mustve Been Ol) Santa Claus*.*" "01-03- (It Mustve Been Ol) Santa Claus"
