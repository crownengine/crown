.. _importing_resources:

Importing resources
===================

Drag-and-drop
-------------

The quickest way to import assets into a Crown project is to drag-and-drop
files from your file manager into the Project Browser. Crown will detect the
resource types automatically and open an appropriate importer dialog so you
can fine-tune import options.

Import via Project Browser
--------------------------

.. figure:: images/project_browser_right_click_import.png
   :align: center

   Importing resources via the Project Browser context menu.

You can also import files from the Project Browser. Navigate to the target
folder, right-click the folder, and choose ``Import...`` from the context
menu. Crown will present a file picker:

.. figure:: images/import_dialog.svg
   :align: center

   The Import dialog for picking files and choosing the resource type.

This method is useful when you want more control: the Import dialog lets you
choose how Crown should process the selected files. That matters because the
same file extension can sometimes map to different resource types
(for example, ``.png`` files can be imported as textures or as sprites). Use
the resource-type selector to pick the desired resource:

.. figure:: images/import_dialog_select_resource_type.svg
   :align: center

   Selecting the resource type in the Import dialog.
