{ pkgs ? import <nixpkgs> { } }:
pkgs.mkShell { inputsFrom = with pkgs; [ qtox ]; }
