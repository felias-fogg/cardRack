if (typeof timestamp === 'undefined' || typeof tickets === 'undefined')
    document.write("Zur Zeit ist es leider nicht möglich, die Anzahl der vorhandenen Monatskarten anzuzeigen, da keine Daten vorliegen.")
else if (timestamp() + 3*3600 > new Date/1000|0) {
    document.write("Es sind zur Zeit ");
    document.write(tickets());
    document.write(" Monatskarten vor Ort vorhanden.");
} else {
    document.write("Zur Zeit ist es leider nicht möglich, die Anzahl der vorhandenen Monatskarten anzuzeigen, da die Daten veraltet sind. ")
}
<!-- reload web page every 60 seconds -->
window.setInterval(function(){
   location.reload(true)
}, 60000);
