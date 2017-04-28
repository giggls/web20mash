// Import process parameters in BeerXML or xsud (kleiner Brauhelfer) format
// This is done by translation input files via xslt stylesheets

function evalXSLT(xslt,doc) {
  xsltProcessor = new XSLTProcessor()
  xsltProcessor.importStylesheet(xslt);
  var result = xsltProcessor.transformToFragment(doc, document).textContent;
  updateSettings(result);  
}

function genconfig(xmlstr) {
  if (document.implementation && document.implementation.createDocument) {
    // try to figure out which input file format we got
    // libmagic for the poor :)
    if ( 0 == xmlstr.search(/^\s*<\?xml .*>[\s\S]*$/i)) {
      var xmlDoc = new DOMParser().parseFromString(xmlstr, 'text/xml');
      if (xmlDoc.documentElement.nodeName == "xsud") {
        $.get( "xslt/xsud.xsl", function(xsl) {
          evalXSLT(xsl,xmlDoc);
        });
      } else {
        if (xmlDoc.documentElement.nodeName == "RECIPES") {
          $.get( "xslt/beerxml.xsl", function(xsl) {
            evalXSLT(xsl,xmlDoc);
          });
        } else {
          alert(i18n.importerror);
          return;
        }
      }
    } else {
      alert(i18n.importerror);
      return;
    }
  }
}

function configread(configfile) {
  var reader = new FileReader();
  reader.onloadend = function(event) { 
    var contents = event.target.result;
    genconfig(contents);
  };
  reader.readAsText(configfile);
}

function updateSettings(settings) {
  var sarr=settings.split(" ")
  var len = sarr.length/2;
  // Bei Anzahl Rasten != 4 heuristische Annahmen treffen
  // Kombrirast
  if (len == 1) {
    $("input[name='resttemp1']").val(0);
    $("input[name='resttime1']").val(0);
    $("input[name='resttemp2']").val(0);
    $("input[name='resttime2']").val(0);
    $("input[name='resttemp3']").val(sarr[0]);
    $("input[name='resttime3']").val(sarr[1]);
    $("input[name='resttemp4']").val(sarr[0]);
    $("input[name='resttime4']").val(0);
    return
  }
  // Ohne Abmaischrast und Einweißrast
  if (len == 2) {
    if ((sarr[0] < 64) && (sarr[2] < 75)) {
      $("input[name='resttemp1']").val(0);
      $("input[name='resttime1']").val(0);
      $("input[name='resttemp2']").val(sarr[0]);
      $("input[name='resttime2']").val(sarr[1]);
      $("input[name='resttemp3']").val(sarr[2]);
      $("input[name='resttime3']").val(sarr[3]);
      $("input[name='resttemp4']").val(sarr[2]);
      $("input[name='resttime4']").val(0);
    } else {
      $("input[name='resttemp1']").val(0);
      $("input[name='resttime1']").val(0);
      $("input[name='resttemp2']").val(0);
      $("input[name='resttime2']").val(0);
      $("input[name='resttemp3']").val(sarr[0]);
      $("input[name='resttime3']").val(sarr[1]);
      $("input[name='resttemp4']").val(sarr[2]);
      $("input[name='resttime4']").val(sarr[3]);
    }
    return
  }
  // Entweder ohne Abmaischrast oder ohne Einweißrast
  // geht also nur heuristisch
  // Annahme: 2 Temperaturen unter 65°C keine Abmaischrast
  if (len == 3) {
    if ((sarr[0] < 68) && (sarr[2] < 68)) {
      $("input[name='resttemp1']").val(sarr[0]);
      $("input[name='resttime1']").val(sarr[1]);
      $("input[name='resttemp2']").val(sarr[2]);
      $("input[name='resttime2']").val(sarr[3]);
      $("input[name='resttemp3']").val(sarr[4]);
      $("input[name='resttime3']").val(sarr[5]);
      $("input[name='resttemp4']").val(sarr[4]);
      $("input[name='resttime4']").val(0);
    } else {
      $("input[name='resttemp1']").val(0);
      $("input[name='resttime1']").val(0);
      $("input[name='resttemp2']").val(sarr[0]);
      $("input[name='resttime2']").val(sarr[1]);
      $("input[name='resttemp3']").val(sarr[2]);
      $("input[name='resttime3']").val(sarr[3]);
      $("input[name='resttemp4']").val(sarr[4]);
      $("input[name='resttime4']").val(sarr[5]); 
    }
    return
  }
  // Alle 4 Rasten definiert
  if (len == 4) {
    $("input[name='resttemp1']").val(sarr[0]);
    $("input[name='resttime1']").val(sarr[1]);
    $("input[name='resttemp2']").val(sarr[2]);
    $("input[name='resttime2']").val(sarr[3]);
    $("input[name='resttemp3']").val(sarr[4]);
    $("input[name='resttime3']").val(sarr[5]);
    $("input[name='resttemp4']").val(sarr[6]);
    $("input[name='resttime4']").val(sarr[7]);
    return
  }                                            
  alert(i18n.importerror);
}


