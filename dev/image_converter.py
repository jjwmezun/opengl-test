from PIL import Image

import sys

def int_to_bytes( value ):
    return value.to_bytes( 2, byteorder='big' )

def convert_file( local_file ):
    full_filename = "dev/images/" + local_file + ".png"
    try:
        image = Image.open( full_filename )
    except FileNotFoundError:
        print( "File not found: %s" %( full_filename ) )
        return -1

    if image.format != "PNG":
        print( "Invalid file format: is %s; must be PNG." %( image.format ) )
        return -1

    palette = image.getpalette()

    if palette == None:
        print( "Invalid file format: missing palette. Must be indexed PNG." )
        return -1

    image = image.transpose( Image.FLIP_TOP_BOTTOM )

    output_data = bytearray()

    width, height = image.size
    width_bytes = int_to_bytes( width )
    height_bytes = int_to_bytes( height )

    output_data.extend( width_bytes )
    output_data.extend( height_bytes )

    data = image.getdata()
    for y in range( height ):
        for x in range( width ):
            item = image.getpixel(( x, y ))
            print( "%s, %s: %s" %( str( x ), str( y ), str( item ) ) )
            color_byte = item.to_bytes( 1, byteorder='big' )
            output_data.extend( color_byte )

    expected_bytes_length = width * height + 4
    bytes_length = len( output_data )
    if bytes_length != expected_bytes_length:
        print( "Computation error: # o’ output bytes doesn’t match expected. Expected %s; have %s" %( str( expected_bytes_length ), str( bytes_length ) ) )
        return -1

    f = open( "bin/" + local_file + ".jwi", "wb" )
    f.write( output_data )
    f.close()

if ( len( sys.argv ) < 2 ):
    print( "Needs a’least 1 argument." )
else:
    for i in range( 1, len( sys.argv ) ):
        convert_file( sys.argv[ i ] )