[![DOI](https://sandbox.zenodo.org/badge/doi/10.7554/eLife.77973.svg)](https://doi.org/10.7554/eLife.77973)

# LabNet

LabNet is a server to control hardware connected to RaspberryPi over Ethernet in behavioral experiment with animals. The protocol uses Google [Protobuf](https://github.com/protocolbuffers/protobuf) for the TCP/IP transport. The main goals are:

1.	Simple communication protocol.
2.	Easy to add support for new hardware.
3.	Low latencies. Commands are mostly executed in 1ms, which includes also the communication over the network.

Build instructions can be found [here](doc/build_instructions.md).

# Scientific Use
You can find the paper online at: https://doi.org/10.7554/eLife.77973

If you use LabNet in a scientific context, please use the following citation:
```bibtex
@article {10.7554/eLife.77973,
    article_type = {journal},
    title = {LabNet hardware control software for the Raspberry Pi},
    author = {Schatz, Alexej and Winter, York},
    editor = {Mathis, Mackenzie W and Wassum, Kate M and Saunders, Jonny L and Lopes, Gonçalo},
    volume = 11,
    year = 2022,
    month = {dec},
    pub_date = {2022-12-30},
    pages = {e77973},
    citation = {eLife 2022;11:e77973},
    doi = {10.7554/eLife.77973},
    url = {https://doi.org/10.7554/eLife.77973},
    keywords = {software, hardware, behaviour},
    journal = {eLife},
    issn = {2050-084X},
    publisher = {eLife Sciences Publications, Ltd},
}
```
