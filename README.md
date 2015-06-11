# object-model-serializer
A tiny C++ library for 'soft serialization' of object models.


#Guiding Principles
    It should be tiny (2 files)
    It should be non-intrusive, procedural style no base class inheritance requirements
    It should internally handle references and cycles ("saving pointers")
    It should be 'soft serialization' ability to add/remove and out-of-order members without 'version blocks'
    It should come with a complex use case example (see simple_rpg folder)
    It should use standard streams, for file and in-memory serialization.
    It should be a lightweight (compared to xml/json) binary format.
    It should not use external pre-processor, or code generation techniques (gross.)
