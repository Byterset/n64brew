void font_init( Gfx ** );
void font_finish( Gfx ** );
void font_show_num( Gfx **, int );
void font_show_string( Gfx **, char * );

void font_set_scale( double, double );
void font_set_color( unsigned char, unsigned char, unsigned char, unsigned char );
void font_set_win( int, int );
void font_set_pos( int, int );

void font_set_transparent( int );

#define SHOWFONT(glp,str,x,y)   {                                             \
                font_set_color( 0,0,0,255); \
                font_set_pos( x, y );                                         \
                font_show_string( glp, str );}