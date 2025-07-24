.. _ant_compatibility:

Compatibility
#############

.. contents::
   :local:
   :depth: 2


SDK Compatibility
*****************

.. note::
  Our deployment model has changed. ANT support is now provided as an nRF Connect SDK Add On. See below for compatibility details.

`ANT for nRF Connect SDK Releases <https://github.com/ant-nrfconnect/sdk-ant/releases>`_ correspond to nRF Connect SDK (sdk-nrf) tagged versions according to the following table:

+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| sdk-nrf         | sdk-ant                                 | Supported SOCs                                                              |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| Version         | Support Model   | Version               | nRF52 Series                  | nRF53 Series         | nRF54 Series         |
+=================+=================+=======================+===============================+======================+======================+
| v2.2            | Manifest        | v0.4.0 (experimental) |                               | nRF5340              |                      |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| v2.3            | Manifest        | v0.5.0 (experimental) |                               | nRF5340              |                      |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| v2.4            | Manifest        | v1.0.0                |                               | nRF5340              |                      |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| v2.5            | Manifest        | v1.1.0                |                               | nRF5340              |                      |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| v2.6            | Manifest        | v1.2.0                | nRF52840                      | nRF5340              |                      |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| v2.7            | Manifest        | v1.3.0                | nRF52840                      | nRF5340              |                      |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| v2.8            | Not supported                                                                                                         |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+
| v2.9.2          | Add-on          | v2.0.0                | nRF52832, nrf52840            | nRF5340              | nRF54L15             |
+-----------------+-----------------+-----------------------+-------------------------------+----------------------+----------------------+


.. note::
  The ANT stack requires a 32.768 kHz low frequency clock source with a maximum tolerance of ±50 ppm.

  On nRF development kits for the SoCs listed above, this means that ANT/ANT+ requires the 32.768 kHz crystal (X2) for correct operation.

Add-on Deployment
*****************

nRF Connect SDK Add-ons are supplementary components that extend the nRF Connect SDK.

In this model, Add-ons specify the compatible revision of sdk-nrf in their own ``west.yml`` manifest file.


Manifest Deployment (<= sdk-nrf v2.7)
*************************************

Since the sdk-ant repository targets specific nRF Connect SDK versions, care should be taken with the versions used to build an application.

Both the nRF Connect SDK and ANT repositories use a main trunk development model. It is recommended that the ``main`` branch should only be used if you are not particularly concerned about stability.

When incompatibilities arise due to changes in sdk-nrf, sdk-ant may be updated in between releases.

Typically, users should choose a Git release tagged version of sdk-nrf and the ANT version that targets that tag. The recommended ANT version should only deviate from the hash in the ``west.yml`` manifest when there is a bugfix or feature preview.

.. figure:: img/ant_nrf_connect_deployment.png
   :alt: ANT for nRF Connect SDK Repository Diagram
   :scale: 50%


