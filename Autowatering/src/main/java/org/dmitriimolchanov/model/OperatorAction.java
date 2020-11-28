package org.dmitriimolchanov.model;

import lombok.*;

@NoArgsConstructor
@AllArgsConstructor
@Setter
@Getter
public class OperatorAction {
    private String forced;
    private String disable;
    private String resume;
}
