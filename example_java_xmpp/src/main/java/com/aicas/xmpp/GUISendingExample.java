/*-----------------------------------------------------------------------*
 * Copyright 2017, aicas GmbH; all rights reserved.
 * This header, including copyright notice, may not be altered or removed.
 *-----------------------------------------------------------------------*/
package com.aicas.xmpp;

import javafx.application.Application;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.Group;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

public class GUISendingExample extends Application
{
  public static void main(String[] args)
  {
    launch(args);
  }

  @Override
  public void start(Stage primaryStage) throws Exception
  {
    Group root = new Group();
    Scene scene = new Scene(root);
    primaryStage.setScene(scene);

    ObservableList<String> joints = FXCollections.observableArrayList("base", "shoulder", "elbow", "wrist");
    ObservableList<String> modes = FXCollections.observableArrayList("NORMAL", "RELATIV");
    VBox base = new VBox();
    ChoiceBox<String> joint = new ChoiceBox<>(joints);
    joint.getSelectionModel().select(0);
    ChoiceBox<String> mode = new ChoiceBox<>(modes);
    mode.getSelectionModel().select(0);
    Label angleLabel = new Label("angle");
    TextField angleText = new TextField("90");
    Label incLabel = new Label("inc");
    TextField incText = new TextField("1");
    Label delayLabel = new Label("delay");
    TextField delayText = new TextField("10");

    Button send = new Button("Send");
    Button setupReceiver = new Button("Setup receiver");

    send.setOnAction(event ->
                       SendRobotData.sendMovement(joint.getValue(),
                                                  mode.getValue(),
                                                  Integer.valueOf(angleText.getText()),
                                                  Integer.valueOf(incText.getText()),
                                                  Integer.valueOf(delayText.getText())));

    setupReceiver.setOnAction(event -> SendRobotData.setupReceiver());
    base.getChildren().addAll(joint, mode, angleLabel, angleText, incLabel, incText, delayLabel, delayText, send, setupReceiver);
    root.getChildren().add(base);

    primaryStage.show();
  }
}
