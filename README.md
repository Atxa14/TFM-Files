# TFM: Estudio de Procedimientos para la Mejora de la QoS mediante Clustering

## Introducción

El presente repositorio contiene los ficheros principales para la **puesta en práctica de la metodología** definida en el Trabajo Fin de Máster. 

Principalmente, dispone de los ficheros necesarios para la **implementación en NS-3 de los escenarios Wi-Fi** definidos a lo largo del estudio teórico y los programas necesarios para la aplicación de **múltiples algoritmos de clustering** sobre la salida de las simulaciones.

De forma adicional, se incluyen **ficheros de apoyo** para el lanzamiento de simulaciones y el manejo de datos y representación gráfica sobre los sets de muestras.

---

## Contenido
A continuación, se presenta la estructura de ficheros del repositorio:

- `/ML`
  - `/Algoritmhms/`
    - `/AP/`
      - `BIRCH_AP.py`
      - `DBSCAN_AP.py`
      - `GMM_AP.py`
      - `KMEANS_AP.py`
    - `/STA/`
      - `BIRCH_STA.py`
      - `DBSCAN_STA.py`
      - `GMM_STA.py`
      - `KMEANS_STA.py`
  - `/Extra/`
    - `change_order.py`
    - `custom_display.py`
  - `/License/`
    - `BSD 3-Clause.txt`
- `/NS-3/`
  - `/Extra/`
    - `/FTP_M2/`
      - `three-gpp-ftp-m2-helper.cc`
      - `three-gpp-ftp-m2-helper.h`
    - `/Helpful_Scripts/`
      - `iterative_run.sh`
  - `/Scenarios/`
    - `scenario1.cc`
    - `scenario2.cc`
    - `scenario3.cc`
    - `scenario4.cc`
  - `/License/`
    - `GPL-2-only.txt`
    
---
    
## *Nota Aclaratoria*

El código relativo a los escenarios de NS-3 y los programas adicionales generados se basan en módulos desarrollados por la CTTC en su proyecto https://gitlab.com/cttc-lena/nr, por lo que requiere la adecuación de la instalación de NS-3 en modo desarrollo según lo indicado en su documentación.

Asimismo, su código se halla licenciado según *GPL-2*, por lo que así se ha indicado en los programas derivados de su proyecto. La licencia correspondiente se puede encontrar en el propio directorio NS-3, y los ficheros licenciados incluyen un comentario con dicha especificación.

Por último, cabe mencionar que el código en Python específico para el tratamiento de los datos de salida de las simmulaciones también hace uso de licencia. En este caso, se utiliza *BSD 3-Clause* debido a su inspiración en extractos del código de Scikit-Learn y sus ejemplos en https://scikit-learn.org/. En consecuencia, se añade la licencia correspondiente en un directorio específico bajo el directorio ML.
