.. _ant_compatibility:

Compatibility
#############

.. contents::
   :local:
   :depth: 2


SDK Compatibility
*****************

`ANT for nRF Connect SDK Releases <https://github.com/ant-nrfconnect/sdk-ant/releases>`_ correspond to nRF Connect SDK (sdk-nrf) tagged versions according to the following table:

+-----------------------+--------------+--------------+--------------+
| sdk-ant Version       | nRF52 Series | nRF53 Series | nRF54 Series |
+=======================+==============+==============+==============+
| v0.4.0 (experimental) | Unsupported  | sdk-nrf v2.2 | N/A          |
+-----------------------+--------------+--------------+--------------+
| v0.5.0 (experimental) | Unsupported  | sdk-nrf v2.3 | N/A          |
+-----------------------+--------------+--------------+--------------+
| v1.0.0                | Unsupported  | sdk-nrf v2.4 | N/A          |
+-----------------------+--------------+--------------+--------------+
| v1.1.0                | Unsupported  | sdk-nrf v2.5 | N/A          |
+-----------------------+--------------+--------------+--------------+
| v1.2.0                |        sdk-nrf v2.6         | Unsupported  |
+-----------------------+-----------------------------+--------------+
| v1.3.0                |        sdk-nrf v2.7         | Unsupported  |
+-----------------------+-----------------------------+--------------+

System on Chip Support
**********************

Within each nRF Series, only select SoCs are supported.

.. list-table::
   :widths: 25 25 25 25
   :header-rows: 1

   * - sdk-ant Version
     - nRF52 Series
     - nRF53 Series
     - nRF54 Series
   * - v1.0.0
     - 
     - nRF5340
     -
   * - v1.1.0
     - 
     - nRF5340
     -
   * - v1.2.0
     - nRF52840
     - nRF5340
     -
   * - v1.3.0
     - nRF52840
     - nRF5340
     -

Deployment
**********

Since the sdk-ant repository targets specific nRF Connect SDK versions, care should be taken with the versions used to build an application.

Both the nRF Connect SDK and ANT repositories use a main trunk development model. It is recommended that the ``main`` branch should only be used if you are not particularly concerned about stability. 

When incompatibilities arise due to changes in sdk-nrf, sdk-ant may be updated in between releases.

Typically, users should choose a Git release tagged version of sdk-nrf and the ANT version that targets that tag. The recommended ANT version should only deviate from the hash in the ``west.yml`` manifest when there is a bugfix or feature preview.

.. figure:: img/ant_nrf_connect_deployment.png
   :alt: ANT for nRF Connect SDK Repository Diagram
   :scale: 50%
