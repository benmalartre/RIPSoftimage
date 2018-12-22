var sSSID = EdamLogin();

var aSeq = EdamGetSequences( sSSID );

var aSeqUI = new Array();
for( var i = 0; i < aSeq.length; i += 2 )
{
    var sSeqFull = aSeq[i];
    var sSeqID   = aSeq[i+1];

    var sSeqName  = sSeqFull.match( /^\w+/i )[0];

    aSeqUI.push( sSeqFull );
    aSeqUI.push( aSeq[i+1] + " " + sSeqName );
}





// sSeq a recuperer dans le parameter.value
var sSeq = aSeqUI[1];

var aSplit = sSeq.split( " ", 2 );
var sSeqID   = aSplit[0];
var sSeqName = aSplit[1];

var aShots = EdamGetSequenceShots( sSSID, sSeqID );

var aShotsUI = new Array();
for( var i = 0; i < aShots.length; i += 2 )
{
    var sShotName = aShots[i];
    LogMessage( "shot dans la " + sSeqName + " : " + sShotName );
    aShotsUI.push( sShotName );
    aShotsUI.push( sShotName );
}


