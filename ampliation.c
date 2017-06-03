/*----------------------------------------------------------------------------*/
/* lee un caracter sin esperar a '\n' y sin eco*/
char getch(){
  int shell_terminal = STDIN_FILENO;
  struct termios conf;
  struct termios conf_new;
  char c;

  tcgetattr(shell_terminal,&conf); /* leemos la configuracion actual */
  conf_new=conf; /* configuramos */
  conf_new.c_lflag&=(~ICANON);  // sin buffer
  conf_new.c_lflag&=(~ECHO);  // sin eco
  conf_new.c_cc[VTIME]=0;
  conf_new.c_cc[VMIN]=1;  /* guardamos la configuracion */
  tcsetattr(shell_terminal,TCSANOW,&conf_new);  /* leemos el caracter */
  c = getc(stdin);
  tcsetattr(shell_terminal,TCSANOW,&conf);  /* restauramos la configuracion */
  return c;
}
/*----------------------------------------------------------------------------*/
/* Los cursores devuelven una secuencia de 3 caracteres, 27 -­‐ 91 -­‐ (65, 66, 67 ó 68) */
int text;
text = getch();
if(text == 27){
  text = getch();
  if(text == 91){
    text = getch();
    if(text == 65){
    /* ARRIBA */
    //mover historial para ARRIBA
    }else if(text == 66){
    /* ABAJO */
    //mover historial para abajo
    }else if(text == 68){
    /* IZQUIERDA */
    //
    }else if(text == 67){
    /* DERECHA */
    //
    }else{
    //
    }}
}else if(text == 127){
/* BORRAR */
}else if{
  //
}
