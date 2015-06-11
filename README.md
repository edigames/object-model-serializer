# object-model-serializer
A tiny C++ library for 'soft serialization' of object models.


#Guiding Principles
    It should be tiny, and therefore approachable.
    It should be non-intrusive, procedural style.
    It should internally handle saving references and cycles.
    It should be 'soft' so you may add/remove/rearrange members without 'version blocks'.
    It should come with a complex use case example (see simple_rpg folder)
    It should use standard streams, for file and in-memory serialization.
    It should be a lightweight (compared to xml/json) binary format.
    It should not rely on pre-processor magic, or code generation techniques.
