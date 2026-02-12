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

.. image:: images/project_browser_right_click_import.png

You can also import files from the Project Browser. Navigate to the target
folder, right-click the folder, and choose ``Import...`` from the context
menu. Crown will present a file picker:

.. image:: images/import_dialog.png

This method is useful when you want more control: the Import dialog lets you
choose how Crown should process the selected files. That matters because the
same file extension can sometimes map to different resource types
(for example, ``.png`` files can be imported as textures or as sprites). Use
the resource-type selector to pick the desired resource:

.. image:: images/import_dialog_select_resource_type.png
