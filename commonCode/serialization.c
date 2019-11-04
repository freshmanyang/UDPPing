/************************************************************************
* File:  serialization.c
*
* Purpose:
*   This contains helper functions for mapping system messages to
*   standard formats that can be sent and received over a network.
*
* Notes:
*
*
************************************************/


char rfc3986[256] = {0};
char html5[256] = {0};

void url_encoder_rfc_tables_init(){

    int i;

    for (i = 0; i < 256; i++){

        rfc3986[i] = isalnum( i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
        html5[i] = isalnum( i) || i == '*' || i == '-' || i == '.' || i == '_' ? i : (i == ' ') ? '+' : 0;
    }
}

char *url_encode( char *table, unsigned char *s, char *enc){

    for (; *s; s++){

        if (table[*s]) sprintf( enc, "%c", table[*s]);
        else sprintf( enc, "%%%02X", *s);
        while (*++enc);
    }

    return( enc);
}

